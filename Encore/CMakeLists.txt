# CMakeList.txt : CMake project for Encore, include source and define
# project specific logic here.
#



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES x86_64 arm64)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(json QUIET)
if (NOT json_FOUND)
  include(FetchContent)
  FetchContent_Declare(
          json
          DOWNLOAD_EXTRACT_TIMESTAMP OFF
          URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
  FetchContent_GetProperties(json)
  if (NOT json_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(json)
  endif()
endif()

option(JSON_Diagnostics "" ON)

set(SDL_X11_XSCRNSAVER OFF CACHE BOOL "" FORCE)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
set(SDLSHADERCROSS_SPIRVCROSS_SHARED OFF)
set(ENABLE_SPIRV_CODEGEN ON)
set(LLVM_ENABLE_WARNINGS OFF)
set(SPIRV_WERROR OFF)
add_definitions(-DENABLE_SPIRV_CODEGEN)
add_subdirectory(thirdparty/sdl EXCLUDE_FROM_ALL)
set(SDLSHADERCROSS_DXC ON)


# Linux is the only platform with pre-compiled dxc binaries with support for SPIR-V.
# TODO: pre-build our own dxc binaries
if (UNIX AND NOT APPLE)
  # Use sdl_shadercross' download script
  set(SDLSHADERCROSS_VENDORED OFF)
  include(thirdparty/sdl_shadercross/build-scripts/download-prebuilt-DirectXShaderCompiler.cmake)
else()
  # Allows DXC to compile correctly
  include(thirdparty/sdl_shadercross/external/DirectXShaderCompiler/cmake/caches/PredefinedParams.cmake)
  set(SDLSHADERCROSS_VENDORED ON)
endif()
#add_subdirectory(thirdparty/spirv-cross)
set(SDLSHADERCROSS_SPIRVCROSS_SHARED OFF)
add_subdirectory(thirdparty/sdl_shadercross EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/glm EXCLUDE_FROM_ALL)


option ( TRACY_ENABLE " " ON )
option ( TRACY_ON_DEMAND " " ON )

add_subdirectory(thirdparty/tracy EXCLUDE_FROM_ALL)

# Add all subdirectories of src

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "include/*.cpp" "include/*.c")
file(GLOB_RECURSE INC_FILES "include/*.h" "include/*.h*" "src/*.h")
# Add source files to the executable
if (WIN32)
  add_executable(Encore ${SRC_FILES} ${INC_FILES} "resources.rc")
else()
  add_executable(Encore ${SRC_FILES} ${INC_FILES})
endif()
# Temp player file

file(COPY "Songs" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
file(COPY "Assets" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
# Set the include directory for the executable
target_include_directories(Encore PRIVATE "include" "src" "include/imgui")
if(WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4) 
    find_library(DISCORD_GAME_SDK NAMES discord_game_sdk PATHS "lib/discord-rpc/windows/x86/")
    file(COPY "lib/discord-rpc/windows/x86/discord_game_sdk.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASS NAMES bass PATHS "lib/bass/windows/x86/")
    file(COPY "lib/bass/windows/x86/bass.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/windows/x86/")
    file(COPY "lib/bass/windows/x86/bassopus.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  endif() 
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(DISCORD_GAME_SDK NAMES discord_game_sdk PATHS "lib/discord-rpc/windows/x64/")
    file(COPY "lib/discord-rpc/windows/x64/discord_game_sdk.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASS NAMES bass PATHS "lib/bass/windows/x64/")
    file(COPY "lib/bass/windows/x64/bass.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/windows/x64/")
    file(COPY "lib/bass/windows/x64/bassopus.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  endif()
  # TODO: Change this when we eventually get pre-compiled binaries for this
  add_custom_command(
          TARGET Encore POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/sdl_shadercross/external/DirectXShaderCompiler/bin/dxcompiler.dll
          ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()
if(UNIX AND NOT APPLE)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(DISCORD_GAME_SDK NAMES discord_game_sdk PATHS "lib/discord-rpc/linux/x64")
    file(COPY "lib/discord-rpc/linux/x64/libdiscord_game_sdk.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASS NAMES bass PATHS "lib/bass/linux/x86_64")
    file(COPY "lib/bass/linux/x86_64/libbass.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/linux/x86_64")
    file(COPY "lib/bass/linux/x86_64/libbassopus.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    file(COPY "thirdparty/sdl_shadercross/external/DirectXShaderCompiler-binaries/linux/lib/libdxcompiler.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
  endif()
endif()
if(APPLE)
  find_library(DISCORD_GAME_SDK NAMES discord_game_sdk PATHS "lib/discord-rpc/macos")
  file(COPY_FILE "lib/discord-rpc/macos/libdiscord_game_sdk.dylib" ${CMAKE_BINARY_DIR}/Encore/discord_game_sdk.dylib)
  find_library(BASS NAMES bass PATHS "lib/bass/macos")
  file(COPY "lib/bass/macos/libbass.dylib" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/macos")
  file(COPY "lib/bass/macos/libbassopus.dylib" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  add_custom_command(TARGET Encore
          POST_BUILD COMMAND
          ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path"
          $<TARGET_FILE:Encore>)
endif()

# Get the current working branch
execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get the latest commit hash
execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "-DENCORE_VERSION=\"v0.2.0\"")

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "-DGIT_BRANCH=\"${GIT_BRANCH}\"")

add_compile_options(-mavx)

set_property(TARGET Encore PROPERTY CXX_STANDARD 20)



target_link_libraries(Encore nlohmann_json::nlohmann_json ${BASS} ${BASSOPUS} ${DISCORD_GAME_SDK} SDL3::SDL3 Tracy::TracyClient SDL3_shadercross::SDL3_shadercross glm::glm)


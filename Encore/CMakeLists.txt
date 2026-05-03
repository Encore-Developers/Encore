# CMakeList.txt : CMake project for Encore, include source and define
# project specific logic here.
#

set(CMAKE_OBJECT_PATH_MAX 500)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES x86_64)
set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)
# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
          URL https://github.com/Encore-Developers/raylib-Source/archive/refs/heads/master.zip
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
  endif()
endif()

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

option(SUPPORT_FILEFORMAT_JPG "Support loading JPG as textures" ON)

# Place Steamworks SDK headers in thirdparty/steam
# Place Steamworks API redistributables in lib/steam
option(STEAM "Compile Steam API support" OFF)
set(SDL_X11_XSCRNSAVER OFF CACHE BOOL "" FORCE)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
set(ENABLE_SPIRV_CODEGEN ON)
add_subdirectory(thirdparty/sdl EXCLUDE_FROM_ALL)

string(TIMESTAMP BUILDDATE UTC)

option ( TRACY_ENABLE " " OFF )
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
target_include_directories(Encore PRIVATE "include" "src" "include/rlImGui" "include/imgui")
if(STEAM)
  target_include_directories(Encore PRIVATE "thirdparty/steam")
endif()
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
    if(STEAM)
      find_library(STEAMAPI NAMES steam_api64 PATHS "lib/steam/win64/")
      file(COPY "lib/steam/win64/steam_api64.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
  endif()
  target_compile_definitions(${PROJECT_NAME} PRIVATE
          "-DGLFW_EXPOSE_NATIVE_WIN32")
endif()
if(UNIX AND NOT APPLE)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(DISCORD_GAME_SDK NAMES discord_game_sdk PATHS "lib/discord-rpc/linux/x64")
    file(COPY "lib/discord-rpc/linux/x64/libdiscord_game_sdk.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASS NAMES bass PATHS "lib/bass/linux/x86_64")
    file(COPY "lib/bass/linux/x86_64/libbass.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/linux/x86_64")
    file(COPY "lib/bass/linux/x86_64/libbassopus.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    if(STEAM)
      find_library(STEAMAPI NAMES steam_api PATHS "lib/steam/linux64/")
      file(COPY "lib/steam/linux64/libsteam_api.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
  endif()
  target_compile_definitions(${PROJECT_NAME} PRIVATE
          "-DGLFW_EXPOSE_NATIVE_X11")
  target_compile_definitions(${PROJECT_NAME} PRIVATE
          "-DGLFW_EXPOSE_NATIVE_WAYLAND")
endif()
if(APPLE)
  find_library(DISCORD_GAME_SDK NAMES discord_game_sdk PATHS "lib/discord-rpc/macos")
  file(COPY_FILE "lib/discord-rpc/macos/libdiscord_game_sdk.dylib" ${CMAKE_BINARY_DIR}/Encore/discord_game_sdk.dylib)
  find_library(BASS NAMES bass PATHS "lib/bass/macos")
  file(COPY "lib/bass/macos/libbass.dylib" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/macos")
  file(COPY "lib/bass/macos/libbassopus.dylib" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  if(STEAM)
    find_library(STEAMAPI NAMES steam_api PATHS "lib/steam/osx/")
    file(COPY "lib/steam/osx/libsteam_api.dylib" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  endif()
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

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "-DBUILDDATE=\"${BUILDDATE}\"")

if(STEAM)
  target_compile_definitions(${PROJECT_NAME} PRIVATE
          "-DSTEAM")
  file(WRITE ${CMAKE_BINARY_DIR}/Encore/steam_appid.txt "4691230")
endif()

set_property(TARGET Encore PROPERTY CXX_STANDARD 20)



target_link_libraries(Encore raylib nlohmann_json::nlohmann_json ${BASS} ${BASSOPUS} ${DISCORD_GAME_SDK} ${STEAMAPI} SDL3::SDL3 Tracy::TracyClient)


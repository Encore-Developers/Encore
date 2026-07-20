# CMakeList.txt : CMake project for Encore, include source and define
# project specific logic here.
#

set(CMAKE_OBJECT_PATH_MAX 500)

set(CMAKE_CXX_STANDARD 20)
message("${CMAKE_SYSTEM_PROCESSOR}")
# Linking now depends on system ffmpeg binaries, which only include whatever matches the local system
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64")
  set(CMAKE_OSX_ARCHITECTURES arm64)
else()
  set(CMAKE_OSX_ARCHITECTURES x86_64)
endif()
set(CMAKE_OSX_DEPLOYMENT_TARGET 13.3)
# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies

include(FetchContent)

find_package(json QUIET)
if (NOT json_FOUND)
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

if (WIN32)
  FetchContent_Declare(ffmpeg DOWNLOAD_EXTRACT_TIMESTAMP OFF URL https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.7z)
  FetchContent_GetProperties(ffmpeg)
  if (NOT ffmpeg_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(ffmpeg)
  endif()

  find_library(AVCODEC_LIBRARY avcodec PATHS "${ffmpeg_SOURCE_DIR}/lib")
  find_library(AVFORMAT_LIBRARY avformat PATHS "${ffmpeg_SOURCE_DIR}/lib")
  find_library(AVUTIL_LIBRARY avutil PATHS "${ffmpeg_SOURCE_DIR}/lib")
  find_library(SWSCALE_LIBRARY swscale PATHS "${ffmpeg_SOURCE_DIR}/lib")
elseif(NOT APPLE)
  find_library(AVCODEC_LIBRARY avcodec)
  find_library(AVFORMAT_LIBRARY avformat)
  find_library(AVUTIL_LIBRARY avutil)
  find_library(SWSCALE_LIBRARY swscale)
else()
  # Requires the ffmpeg package from Homebrew.
  execute_process(
          COMMAND bash "-c" "dirname $(readlink -f $(which ffmpeg))"
          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          OUTPUT_VARIABLE FFMPEG_MACOS_BINARY_PATH
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  find_library(AVCODEC_LIBRARY avcodec PATHS "${FFMPEG_MACOS_BINARY_PATH}/../lib")
  find_library(AVFORMAT_LIBRARY avformat PATHS "${FFMPEG_MACOS_BINARY_PATH}/../lib")
  find_library(AVUTIL_LIBRARY avutil PATHS "${FFMPEG_MACOS_BINARY_PATH}/../lib")
  find_library(SWSCALE_LIBRARY swscale PATHS "${FFMPEG_MACOS_BINARY_PATH}/../lib")

endif()



option(JSON_Diagnostics "" ON)


# Place Steamworks SDK headers in thirdparty/steam
# Place Steamworks API redistributables in lib/steam
option(STEAM "Compile Steam API support" OFF)
set(SDL_X11_XSCRNSAVER OFF CACHE BOOL "" FORCE)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
set(ENABLE_SPIRV_CODEGEN ON)
add_subdirectory(thirdparty/sdl EXCLUDE_FROM_ALL)

set(CUSTOMIZE_BUILD ON)
set(SUPPORT_FILEFORMAT_JPG ON)
set(SUPPORT_MODULE_RAUDIO OFF)
set(PLATFORM "SDL")
add_subdirectory(thirdparty/raylib EXCLUDE_FROM_ALL)

string(TIMESTAMP BUILDDATE UTC)

option ( TRACY_ENABLE " " OFF )
option ( TRACY_ON_DEMAND " " ON )
add_subdirectory(thirdparty/tracy EXCLUDE_FROM_ALL)

ExternalProject_Add(spdlog
        PREFIX spdlog
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/Encore/thirdparty/spdlog
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/Encore/
        -DSPDLOG_BUILD_SHARED=OFF
)

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
file(COPY "thirdparty/opensource/base" DESTINATION ${CMAKE_BINARY_DIR}/Encore/Assets/opensource)
file(COPY "thirdparty/opensource/extra" DESTINATION ${CMAKE_BINARY_DIR}/Encore/Assets/opensource)
# Set the include directory for the executable
add_dependencies(${PROJECT_NAME} spdlog)
target_include_directories(Encore PRIVATE "include" "src" "include/rlImGui" "include/imgui")
# target_include_directories(Encore PUBLIC ${STAGING_DIR}/include/)
if(STEAM)
  target_include_directories(Encore PRIVATE "thirdparty/steam")
endif()
if(WIN32)
  target_include_directories(Encore PRIVATE "${ffmpeg_SOURCE_DIR}/include")
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

  file(COPY "${ffmpeg_SOURCE_DIR}/bin/avcodec-62.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  file(COPY "${ffmpeg_SOURCE_DIR}/bin/avformat-62.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  file(COPY "${ffmpeg_SOURCE_DIR}/bin/avutil-60.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  file(COPY "${ffmpeg_SOURCE_DIR}/bin/swscale-9.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  file(COPY "${ffmpeg_SOURCE_DIR}/bin/swresample-6.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  target_compile_definitions(${PROJECT_NAME} PRIVATE
          "-DGLFW_EXPOSE_NATIVE_WIN32")
endif()
if(UNIX AND NOT APPLE)
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
  target_compile_definitions(${PROJECT_NAME} PRIVATE
          "-DGLFW_EXPOSE_NATIVE_X11")
  target_compile_definitions(${PROJECT_NAME} PRIVATE
          "-DGLFW_EXPOSE_NATIVE_WAYLAND")
endif()
if(APPLE)
  target_include_directories(Encore PRIVATE "${FFMPEG_MACOS_BINARY_PATH}/../include")
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

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  if( supported )
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET Encore PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(STATUS "IPO / LTO not supported: <${error}>")
  endif()
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


#target_include_directories(SDL3-static PUBLIC "include/gameinput")
target_link_libraries(Encore raylib nlohmann_json::nlohmann_json ${BASS} ${BASSOPUS} ${DISCORD_GAME_SDK} ${STEAMAPI} SDL3::SDL3 Tracy::TracyClient ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})


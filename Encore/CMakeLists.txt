# CMakeList.txt : CMake project for Encore, include source and define
# project specific logic here.
#



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES x86_64)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
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

if(WIN32)
FetchContent_Declare(
zlib
GIT_REPOSITORY https://github.com/madler/zlib.git
GIT_TAG v1.3.1
)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "Z" FORCE)
FetchContent_MakeAvailable(zlib)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/scripts/setup-ffmpeg.cmake)
setup_ffmpeg()
find_ffmpeg_libraries()



option(SUPPORT_FILEFORMAT_JPG "Support loading JPG as textures" ON)
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
target_include_directories(Encore PRIVATE "include" "src" ${FFMPEG_INCLUDE_DIR})
if(WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4) 
    find_library(DISCORD_GAME_SDK NAMES discord_game_sdk PATHS "lib/discord-rpc/windows/x86/")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/discord-rpc/windows/x86/discord_game_sdk.dll")
      file(COPY "lib/discord-rpc/windows/x86/discord_game_sdk.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
    find_library(BASS NAMES bass PATHS "lib/bass/windows/x86/")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/bass/windows/x86/bass.dll")
      file(COPY "lib/bass/windows/x86/bass.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
    find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/windows/x86/")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/bass/windows/x86/bassopus.dll")
      file(COPY "lib/bass/windows/x86/bassopus.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
  endif() 
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(DISCORD_GAME_SDK NAMES discord_game_sdk PATHS "lib/discord-rpc/windows/x64/")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/discord-rpc/windows/x64/discord_game_sdk.dll")
      file(COPY "lib/discord-rpc/windows/x64/discord_game_sdk.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
    find_library(BASS NAMES bass PATHS "lib/bass/windows/x64/")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/bass/windows/x64/bass.dll")
      file(COPY "lib/bass/windows/x64/bass.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
    find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/windows/x64/")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/bass/windows/x64/bassopus.dll")
      file(COPY "lib/bass/windows/x64/bassopus.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
  endif()
endif()
if(UNIX AND NOT APPLE)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(DISCORD_GAME_SDK NAMES discord_game_sdk PATHS "lib/discord-rpc/linux/x64")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/discord-rpc/linux/x64/libdiscord_game_sdk.so")
      file(COPY "lib/discord-rpc/linux/x64/libdiscord_game_sdk.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
    find_library(BASS NAMES bass PATHS "lib/bass/linux/x86_64")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/bass/linux/x86_64/libbass.so")
      file(COPY "lib/bass/linux/x86_64/libbass.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
    find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/linux/x86_64")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/bass/linux/x86_64/libbassopus.so")
      file(COPY "lib/bass/linux/x86_64/libbassopus.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    endif()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
  endif()
endif()
if(APPLE)
  find_library(DISCORD_GAME_SDK NAMES discord_game_sdk PATHS "lib/discord-rpc/macos")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/discord-rpc/macos/libdiscord_game_sdk.dylib")
    file(COPY "lib/discord-rpc/macos/libdiscord_game_sdk.dylib" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  endif()
  find_library(BASS NAMES bass PATHS "lib/bass/macos")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/bass/macos/libbass.dylib")
    file(COPY "lib/bass/macos/libbass.dylib" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  endif()
  find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/macos")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/bass/macos/libbassopus.dylib")
    file(COPY "lib/bass/macos/libbassopus.dylib" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
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

set_property(TARGET Encore PROPERTY CXX_STANDARD 20)


set(ENCORE_LIBRARIES
raylib
nlohmann_json::nlohmann_json
${BASS}
${BASSOPUS}
${DISCORD_GAME_SDK}
${FFMPEG_LIBRARIES}
)
if(UNIX AND NOT APPLE)
find_package(ZLIB REQUIRED)
list(APPEND ENCORE_LIBRARIES ZLIB::ZLIB)
endif()
if(APPLE)
find_package(ZLIB REQUIRED)
list(APPEND ENCORE_LIBRARIES ZLIB::ZLIB)


find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
find_library(COREMEDIA_FRAMEWORK CoreMedia)
find_library(COREVIDEO_FRAMEWORK CoreVideo)
find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
find_library(VIDEOTOOLBOX_FRAMEWORK VideoToolbox)
find_library(SECURITY_FRAMEWORK Security)
find_library(APPKIT_FRAMEWORK AppKit)
find_library(IOKIT_FRAMEWORK IOKit)

list(APPEND ENCORE_LIBRARIES
${COREFOUNDATION_FRAMEWORK}
${COREMEDIA_FRAMEWORK}
${COREVIDEO_FRAMEWORK}
${AUDIOTOOLBOX_FRAMEWORK}
${VIDEOTOOLBOX_FRAMEWORK}
${SECURITY_FRAMEWORK}
${APPKIT_FRAMEWORK}
${IOKIT_FRAMEWORK}
)

list(APPEND ENCORE_LIBRARIES "-liconv" "-lbz2" "-llzma")
endif()
if(WIN32)
list(APPEND ENCORE_LIBRARIES zlibstatic bcrypt)
endif()
if(UNIX AND NOT APPLE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-fatal-warnings")
endif()
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
endif()

target_link_libraries(Encore PRIVATE ${ENCORE_LIBRARIES})

if(UNIX AND NOT APPLE)
set_target_properties(Encore PROPERTIES
BUILD_WITH_INSTALL_RPATH TRUE
INSTALL_RPATH "$ORIGIN"
INSTALL_RPATH_USE_LINK_PATH FALSE
)
endif()

# Set up runtime output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Encore/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Encore/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/Encore/RelWithDebInfo)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/Encore/MinSizeRel)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Encore/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Encore/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/Encore/RelWithDebInfo)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/Encore/MinSizeRel)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Encore/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Encore/Release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/Encore/RelWithDebInfo)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/Encore/MinSizeRel)

if(NOT CMAKE_CONFIGURATION_TYPES)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Encore/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Encore/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Encore/${CMAKE_BUILD_TYPE})
endif()

if(CMAKE_CONFIGURATION_TYPES)
setup_all_post_build(Encore ${CMAKE_BINARY_DIR}/Encore/$<CONFIG>)
else()
setup_all_post_build(Encore ${CMAKE_BINARY_DIR}/Encore/${CMAKE_BUILD_TYPE})
endif()




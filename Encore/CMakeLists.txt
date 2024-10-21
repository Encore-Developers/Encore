# CMakeList.txt : CMake project for Encore, include source and define
# project specific logic here.
#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES x86_64;arm64)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Dependencies
set(RAYLIB_VERSION 5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/Encore-Developers/raylib/archive/refs/heads/master.zip
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

option(SUPPORT_FILEFORMAT_JPG "Support loading JPG as textures" ON)
# Add all subdirectories of src

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "include/*.cpp" "include/*.c")
file(GLOB_RECURSE INC_FILES "include/*.h" "include/*.h*" "src/*.h")
# Add source files to the executable
add_executable(Encore ${SRC_FILES} ${INC_FILES})
# Temp player file

file(COPY "Songs" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
file(COPY "Assets" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
# Set the include directory for the executable
target_include_directories(Encore PRIVATE "include" "src")
if(WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4) 
    find_library(BASS NAMES bass PATHS "lib/bass/windows/x86/")
    file(COPY "lib/bass/windows/x86/bass.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/windows/x86/")
    file(COPY "lib/bass/windows/x86/bassopus.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  endif() 
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(BASS NAMES bass PATHS "lib/bass/windows/x64/")
    file(COPY "lib/bass/windows/x64/bass.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/windows/x64/")
    file(COPY "lib/bass/windows/x64/bassopus.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  endif()
endif()
if(UNIX AND NOT APPLE)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(BASS NAMES bass PATHS "lib/bass/linux/x86_64")
    file(COPY "lib/bass/linux/x86_64/libbass.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/linux/x86_64")
    file(COPY "lib/bass/linux/x86_64/libbassopus.so" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
  endif()
endif()
if(APPLE)
  find_library(BASS NAMES bass PATHS "lib/bass/macos")
  file(COPY "lib/bass/macos/libbass.dylib" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  find_library(BASSOPUS NAMES bassopus PATHS "lib/bass/macos")
  file(COPY "lib/bass/macos/libbassopus.dylib" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
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

target_link_libraries(Encore raylib nlohmann_json::nlohmann_json ${BASS} ${BASSOPUS})

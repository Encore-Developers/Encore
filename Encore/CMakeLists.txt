# CMakeList.txt : CMake project for Encore, include source and define
# project specific logic here.
#

set(CMAKE_CXX_STANDARD 20)
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
    URL https://github.com/NarrikSynthfox/raylib/archive/refs/heads/master.zip
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

option(SUPPORT_FILEFORMAT_JPG "Support loading JPG as textures" ON)
# Add all subdirectories of src
file(GLOB_RECURSE SRC_FILES "src/*.cpp")
file(GLOB_RECURSE INC_FILES "include/*.h" "src/*.h")
# Add source files to the executable
add_executable(Encore ${SRC_FILES} ${INC_FILES})
file(COPY "Songs" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
file(COPY "Assets" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
# Set the include directory for the executable
target_include_directories(Encore PRIVATE "include")
if(WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4) 
    find_library(BASS NAMES bass PATHS "lib/bass/x86/")
    file(COPY "lib/bass/x86/bass.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  endif() 
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(BASS NAMES bass PATHS "lib/bass/x64/")
    file(COPY "lib/bass/x64/bass.dll" DESTINATION ${CMAKE_BINARY_DIR}/Encore)
  endif()
endif()
if(UNIX AND NOT APPLE)
    find_library(BASS NAMES bass PATHS "lib/bass/linux/")
endif()
target_link_libraries(Encore raylib ${BASS})
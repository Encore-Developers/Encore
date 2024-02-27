# CMakeList.txt : CMake project for FNFestClone, include source and define
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
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

# Add all subdirectories of src
file(GLOB_RECURSE SRC_FILES "src/*.cpp")
file(GLOB_RECURSE INC_FILES "include/*.h" "src/*.h")
# Add source files to the executable
add_executable(FNFestClone ${SRC_FILES} ${INC_FILES})
file(COPY "Songs" DESTINATION ${CMAKE_BINARY_DIR}/FNFestClone)
file(COPY "Assets" DESTINATION ${CMAKE_BINARY_DIR}/FNFestClone)
# Set the include directory for the executable
target_include_directories(FNFestClone PRIVATE "include")
# Link against the DLLs in the lib directory
target_link_libraries(FNFestClone raylib)
# TODO: Add tests and install targets if needed.

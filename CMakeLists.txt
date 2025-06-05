cmake_minimum_required(VERSION 3.11)

if(PLATFORM_NX)
  set(DEVKITPRO /opt/devkitpro)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain.cmake")
endif()

project(Encore LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

if(NOT PLATFORM_NX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2" CACHE STRING "C flags")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2" CACHE STRING "C++ flags")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(Encore)
cmake_minimum_required(VERSION 3.11)

file(GLOB SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.c"
)
file(GLOB INC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(Encore ${SRC_FILES} ${INC_FILES}
        src/EncoreLog.h)

set_property(TARGET Encore PROPERTY CXX_STANDARD 20)

if(PLATFORM_NX)
  set(RAYLIB_NX_PATH /mnt/c/Users/jayde/build-switch/raylib-nx)
  set(DEVKITPRO /opt/devkitpro)

  if(NOT EXISTS "${RAYLIB_NX_PATH}/src/raylib.h")
    message(FATAL_ERROR "raylib.h not found at ${RAYLIB_NX_PATH}/src")
  endif()
  if(NOT EXISTS "${RAYLIB_NX_PATH}/src/libraylib.a")
    message(FATAL_ERROR "libraylib.a not found at ${RAYLIB_NX_PATH}/src")
  endif()

  target_include_directories(Encore PRIVATE
          "${RAYLIB_NX_PATH}/src"
          "${DEVKITPRO}/libnx/include"
          "${DEVKITPRO}/portlibs/switch/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

  target_link_libraries(Encore PRIVATE
          "${RAYLIB_NX_PATH}/src/libraylib.a"
          "${DEVKITPRO}/libnx/lib/libnx.a"
          -pthread
  )

  target_compile_definitions(Encore PRIVATE
          PLATFORM_NX
          ENCORE_VERSION="1.0.0"
          GIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
  )

  find_package(Git)
  if(GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  else()
    set(GIT_COMMIT_HASH "unknown")
  endif()

  set_target_properties(Encore PROPERTIES SUFFIX ".elf")

  add_custom_command(TARGET Encore POST_BUILD
          COMMAND "${DEVKITPRO}/tools/bin/elf2nro"
          "${CMAKE_CURRENT_BINARY_DIR}/Encore.elf"
          "${CMAKE_BINARY_DIR}/Encore.nro"
          COMMENT "Converting to NRO"
  )

  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Songs" DESTINATION "${CMAKE_BINARY_DIR}/Encore")
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Assets" DESTINATION "${CMAKE_BINARY_DIR}/Encore")
else()
  include(FetchContent)
  FetchContent_Declare(
          raylib
          GIT_REPOSITORY https://github.com/raysan5/raylib.git
          GIT_TAG 5.0
  )
  FetchContent_MakeAvailable(raylib)
  target_include_directories(Encore PRIVATE
          "${CMAKE_CURRENT_SOURCE_DIR}/src"
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
  target_link_libraries(Encore PRIVATE raylib)

  target_compile_definitions(Encore PRIVATE
          ENCORE_VERSION="1.0.0"
          GIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
  )

  find_package(Git)
  if(GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  else()
    set(GIT_COMMIT_HASH "unknown")
  endif()
endif()

set(JSON_INCLUDE_DIR /mnt/c/Users/jayde/build-switch/deps/nlohmann_json/include)
if(NOT EXISTS "${JSON_INCLUDE_DIR}/nlohmann/json.hpp")
  message(FATAL_ERROR "nlohmann/json.hpp not found at ${JSON_INCLUDE_DIR}")
endif()
target_include_directories(Encore PRIVATE "${JSON_INCLUDE_DIR}")

option(SUPPORT_FILEFORMAT_JPG "Support JPG textures" ON)
if(SUPPORT_FILEFORMAT_JPG)
  target_compile_definitions(Encore PRIVATE SUPPORT_FILEFORMAT_JPG)
endif()
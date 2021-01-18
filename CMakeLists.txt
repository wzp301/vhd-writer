cmake_minimum_required(VERSION 2.10)

project (vhd-writer CXX)

set (${PROJECT_NAME}_VERSION_MAJOR 1)
set (${PROJECT_NAME}_VERSION_MINOR 0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJECT_BIN "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_CXX_STANDARD 11)

if(${Tag_LINUX})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Werror -Wall -Wno-error=format-zero-length -Wno-unused-variable -Wno-format-security -Wno-unused-function -Wno-sign-conversion")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BIN}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BIN}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BIN}")

add_executable(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/src/main.cpp")
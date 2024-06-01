cmake_minimum_required(VERSION 3.10)

# Project name
project(DXFramework C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

set(SOURCE_FILES src/main.c)

add_executable(DXFramework ${SOURCE_FILES})
target_link_libraries(DXFramework m asound)
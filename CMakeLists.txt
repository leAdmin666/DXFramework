cmake_minimum_required(VERSION 3.10)

# Project name
project(DXFramework)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

set(SOURCE_FILES src/main.cpp)

# vulkan_tutorial executable
add_executable(DXFramework ${SOURCE_FILES})

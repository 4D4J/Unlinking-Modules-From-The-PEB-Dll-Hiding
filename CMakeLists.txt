cmake_minimum_required(VERSION 3.15)
project(PebUnlinking CXX)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")

add_library(PebUnlinkDll SHARED Source.cpp Prototypes.h)


set_target_properties(PebUnlinkDll PROPERTIES OUTPUT_NAME "exemple")

add_executable(TestApp TestApp.cpp)

cmake_minimum_required(VERSION 3.14)
project(tradigitools LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_CXX_STANDARD 17 )

add_subdirectory(maya)

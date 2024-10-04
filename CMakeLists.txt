cmake_minimum_required(VERSION 3.20)
project(
    "main"
    VERSION 1.0
    DESCRIPTION ""
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(main)
target_sources(main PRIVATE src/main.cpp)
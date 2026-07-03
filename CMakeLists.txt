cmake_minimum_required(VERSION 3.10)
project(image_processor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    image_processor
    image_processor.cpp
    bmp_io.cpp
    filters.cpp
    matrix_filter.cpp
)

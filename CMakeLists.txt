cmake_minimum_required(VERSION 3.16)
project(mathml2omml VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mathml2omml STATIC
    mathml2omml.cpp
)

target_include_directories(mathml2omml PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    add_subdirectory(tests)
endif()

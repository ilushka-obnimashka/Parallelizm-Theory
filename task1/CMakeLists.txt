cmake_minimum_required(VERSION 3.22)
project(task1)

SET(CMAKE_CXX_STANDARD 17)

if (DEFINED USE_DOUBLE)
    add_definitions(-DUSE_DOUBLE)
elseif (DEFINED USE_FLOAT)
    add_definitions(-DUSE_FLOAT)
else()
    message(FATAL_ERROR "It is necessary to determine USE_DOUBLE or USE_FLOAT.")
endif ()

add_executable(task1 main.cpp)
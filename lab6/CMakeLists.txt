cmake_minimum_required(VERSION 3.22)
project(lab6 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0167 OLD)

# Для C-таргета явно указываем необходимость линковки с libm
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Boost 1.74 COMPONENTS program_options)

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Boost 1.74+ не найден...")
endif ()

# C++ версия с Boost
add_executable(lab6_cpp
        main.cpp
        all_classes.h
)

target_link_libraries(lab6_cpp PRIVATE
        Boost::program_options
)

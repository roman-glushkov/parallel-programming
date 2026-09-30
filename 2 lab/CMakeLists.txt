cmake_minimum_required(VERSION 3.15)
project(blur_lab2 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(blur main.cpp)

# stb — header-only, только путь к заголовкам
target_include_directories(blur PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# оптимизация для g++/clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(blur PRIVATE -O2 -Wall -Wextra)
endif()
cmake_minimum_required(VERSION 3.15)
project(lab3 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(COMMON_SOURCES bank.cpp wallet.cpp)

add_executable(bank main.cpp ${COMMON_SOURCES})
add_executable(test_bank test_bank.cpp ${COMMON_SOURCES})

enable_testing()

add_test(NAME bank_tests       COMMAND test_bank)
add_test(NAME economy_single   COMMAND bank --mode single   --seconds 1 --quiet)
add_test(NAME economy_parallel COMMAND bank --mode parallel --seconds 1 --quiet)
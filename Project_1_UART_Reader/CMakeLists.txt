cmake_minimum_required(VERSION 3.16)
project(UART_Reader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(UART_Reader ${SRCS})
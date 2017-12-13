cmake_minimum_required(VERSION 3.8)
project(roomba_cpp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        main.cpp
        Roomba.cpp
        Roomba.h)

add_executable(roomba_cpp ${SOURCE_FILES})
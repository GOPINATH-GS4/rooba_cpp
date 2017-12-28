cmake_minimum_required(VERSION 3.0)
project(roomba_cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        main.cpp
        Roomba.cpp
        Roomba.h)

set(SERVER Server.cpp Roomba.h Roomba.cpp)
set(CLIENT Client.cpp)

add_executable(roomba_cpp ${SOURCE_FILES})
add_executable(server ${SERVER})
add_executable(client ${CLIENT})
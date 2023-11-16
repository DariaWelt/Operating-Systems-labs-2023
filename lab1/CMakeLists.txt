cmake_minimum_required(VERSION 3.10)
project(reminder)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

add_executable(reminder main.cpp Reminder.cpp)

cmake_minimum_required(VERSION 3.10)
project(ex01 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(ex01 src/main.cpp)

target_link_libraries(ex01 ${OpenCV_LIBS})
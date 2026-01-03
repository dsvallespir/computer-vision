cmake_minimum_required(VERSION 3.10)
project(PointerDetector)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})

# Create executables
add_executable(pointer_detector src/pointer_detector.cpp)
add_executable(simple_pointer src/simple_pointer.cpp)

# Link OpenCV libraries
target_link_libraries(pointer_detector ${OpenCV_LIBS})
target_link_libraries(simple_pointer ${OpenCV_LIBS})

# Installation (optional)
install(TARGETS pointer_detector simple_pointer DESTINATION bin)
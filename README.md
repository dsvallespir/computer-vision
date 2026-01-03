# Computer Vision

## Preparing workspace
```bash
sudo apt update
sudo apt upgrade -y
sudo apt install build-essenstial git cmake pkg-config -y
sudo apt install libopencv-dev python3-opencv -y
sudo apt install ffmpeg gstraemer1.0-tools v4l-utils -y
```

## Verify install
```bash
pkg-config --modversion opencv4
```

## See available cameras:
```bash
v4l2-ctl --list-devices
```

## View supported formats:
```bash
v4l2-ctl -d /dev/video0 --list-formats-ext
```

## Standard CMakeLists.txt

```bash
cmake_minimum_required(VERSION 3.10)
project(cv_base LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(cv_base src/main.cpp)

target_link_libraries(cv_base ${OpenCV_LIBS})

```
## Compiling

```bash
cmake ..
make -j4
```

## Push changes to remote
```bash
git push https://github.com/dsvallespir/computer-vision main
```
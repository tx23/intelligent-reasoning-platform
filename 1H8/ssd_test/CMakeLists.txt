cmake_minimum_required(VERSION 2.8)

project(edu_expr)
set( CMAKE_CXX_FLAGS "-std=c++11" )

#set cross-compile
set(PROJECT_ROOT_DIR "/home/archlab/work/HCAI1H_HOST_SDK")
set(CROSS_COMPILE_DIR "${PROJECT_ROOT_DIR}/toolchain/arm-linux-gnueabihf-4.8.3-201404/bin")
set(CMAKE_CXX_COMPILER "${CROSS_COMPILE_DIR}/arm-linux-gnueabihf-g++")

#set output path
set(EXECUTABLE_OUTPUT_PATH ../)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../)

include_directories("${PROJECT_ROOT_DIR}/include")
include_directories("./src")
link_directories("${PROJECT_ROOT_DIR}/lib/arm")

#.so file
add_library(ssddetect SHARED ./src/ssd_offline.cpp)
target_link_libraries(ssddetect opencv_imgcodecs opencv_imgproc opencv_core opencv_highgui protobuf glog gflags pthread stdc++ cnrt)

#test 
add_executable(ssd_test.bin ./src/main.cpp)
target_link_libraries(ssd_test.bin opencv_imgcodecs opencv_imgproc opencv_core opencv_highgui protobuf glog gflags pthread stdc++ ssddetect)

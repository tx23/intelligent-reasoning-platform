cmake_minimum_required(VERSION 2.8.0)

## project 路径
set(PROJECT_PTH ${CMAKE_CURRENT_SOURCE_DIR}/../)

### SET CROSS-COMPILE ###
set(CMAKE_SYSTEM_NAME Linux)
## set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE DEBUG)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


set(TOOLCHAIN_ROOT /home/archlab/1h8/HCAI1H_HOST_SDK/toolchain/arm-linux-gnueabihf-4.8.3-201404/bin)
set(CMAKE_C_COMPILER ${TOOLCHAIN_ROOT}/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_ROOT}/arm-linux-gnueabihf-g++)
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_ROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(MODE_FILE "../model/ssd.cambricon")
###

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -mfloat-abi=hard -mfpu=neon")
set(EXECUTABLE_OUTPUT_PATH ../../run)
add_definitions(-DBUILD_WITH_DLA)

include_directories(${PROJECT_PTH}/include)
include_directories(${PROJECT_PTH}/include/detection)
include_directories(${PROJECT_PTH}/src)

link_directories(${PROJECT_PTH}/lib/opencv)
link_directories(${PROJECT_PTH}/lib)

add_executable(transaction_process task_main.cpp system_check.cpp task_requester.cpp communicate.cpp)
target_link_libraries(transaction_process pthread opencv_imgproc opencv_core opencv_highgui protobuf glog gflags cnrt ssddetect)

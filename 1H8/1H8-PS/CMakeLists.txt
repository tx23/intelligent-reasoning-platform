cmake_minimum_required(VERSION 2.8.0)


## project 路径
set(PROJECT_PTH ${CMAKE_CURRENT_SOURCE_DIR}/../)
## 第三方库 路径
set(THIRD_PARTY_PTH ${PROJECT_PTH}/third_party/)

### SET CROSS-COMPILE ###
set(CMAKE_SYSTEM_NAME Linux)
#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE DEBUG)

#set(PROJECT_ROOT_DIR "/home/archlab001/Code/Code/PS")
#set(CROSS_COMPILE_DIR "${PROJECT_ROOT_DIR}/toolchain/aarch64-linux/bin")
#set(CMAKE_CXX_COMPILER "${CROSS_COMPILE_DIR}/aarch64-linux-gnu-g++")

#set(TOOLCHAIN_ROOT /opt/ToolChain/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux)

#set(CMAKE_CXX_COMPILER g++)
#set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
set(EXECUTABLE_OUTPUT_PATH ../../run)
add_definitions(-DBUILD_WITH_DLA)

#include_directories(${PROJECT_PTH}/boost_1_58)
#include_directories(${PROJECT_PTH}/schedule/ ${THIRD_PARTY_PTH}/opencv_aar/include)
#link_directories(${THIRD_PARTY_PTH}/opencv_aar/lib)
#link_directories(${PROJECT_PTH}/lib_boost)


include_directories( ${PROJECT_PTH}/include)
include_directories( ${PROJECT_PTH}/schedule)
include_directories( ${THIRD_PARTY_PTH}/opencv/include)
link_directories(${THIRD_PARTY_PTH}/opencv/lib)

#include_directories( ${THIRD_PARTY_PTH}/opencv_aar/include)
#link_directories(${THIRD_PARTY_PTH}/opencv_aar/lib)

add_executable(transaction_process task_main.cpp task_schedule.cpp communicate.cpp blocking_queue.cpp task_agency.cpp task_readimg.cpp data_recv.cpp)


target_link_libraries(transaction_process  pthread ${mst_libs} opencv_imgcodecs opencv_imgproc opencv_core opencv_highgui opencv_videoio pthread boost_atomic boost_system boost_thread api stdc++ -ldl)




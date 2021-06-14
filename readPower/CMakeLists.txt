cmake_minimum_required (VERSION 2.8)

project (powerRead)

link_directories("${PROJECT_SOURCE_DIR}/lib")


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

add_executable(powerRead ./main.c )
target_link_libraries(powerRead cndev)

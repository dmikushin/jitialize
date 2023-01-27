cmake_minimum_required(VERSION 3.4.3)

project(test)

find_package(Jitialize REQUIRED CONFIG)

message("Jitialize include dir: " ${Jitialize_INCLUDE_DIRS})
message("Jitialize lib dir: " ${Jitialize_LIBRARY_DIRS})
message("Jitialize runtime: " ${Jitialize_LIBRARY})
message("Jitialize plugin: " ${Jitialize_PLUGIN})

include_directories(${Jitialize_INCLUDE_DIRS})
link_directories(${Jitialize_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -Xclang -disable-O0-optnone -Xclang -load -Xclang ${Jitialize_PLUGIN}")

add_executable(InstallTest
  test.cpp
)

target_link_libraries(InstallTest ${Jitialize_LIBRARY})

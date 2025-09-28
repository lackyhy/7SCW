# cmake_minimum_required(VERSION <specify CMake version here>)
cmake_minimum_required(VERSION 3.16)
project(7SCW)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

add_executable(7SCW main.cpp terminal_commands.cpp terminal_commands.h SHOW_ALL_STARTUP.cpp SHOW_ALL_STARTUP.h)
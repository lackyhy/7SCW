# cmake_minimum_required(VERSION <specify CMake version here>)
cmake_minimum_required(VERSION 3.16)
project(7SCW)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

add_executable(7SCW main.cpp cpp_file/terminal/terminal_commands.cpp h_file/terminal/terminal_commands.h cpp_file/startup/SHOW_ALL_STARTUP.cpp h_file/startup/SHOW_ALL_STARTUP.h cpp_file/file_manager/file_manager.cpp h_file/file_manager/file_manager.h cpp_file/startup/restoreStartupSettings.cpp h_file/startup/restoreStartupSettings.h cpp_file/terminal/terminal.cpp h_file/terminal/terminal.h cpp_file/startup/startup.cpp h_file/startup/startup.h h_file/main.h)
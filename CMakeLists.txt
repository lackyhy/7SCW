# cmake_minimum_required(VERSION <specify CMake version here>)
cmake_minimum_required(VERSION 3.16)
project(7SCW)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

add_executable(7SCW main.cpp
        include/h_file/main.h

        include/cpp_file/argv.cpp
        include/h_file/argv.h

        include/cpp_file/terminal/terminal_commands.cpp
        include/h_file/terminal/terminal_commands.h

        include/cpp_file/startup/SHOW_ALL_STARTUP.cpp
        include/h_file/startup/SHOW_ALL_STARTUP.h

        include/cpp_file/file_manager/file_manager.cpp
        include/h_file/file_manager/file_manager.h

        include/cpp_file/startup/restoreStartupSettings.cpp
        include/h_file/startup/restoreStartupSettings.h

        include/cpp_file/terminal/terminal.cpp
        include/h_file/terminal/terminal.h

        include/cpp_file/startup/startup.cpp
        include/h_file/startup/startup.h

        include/cpp_file/terminal/speed_test.cpp
        include/h_file/terminal/speed_test.h

        include/cpp_file/security/file_hash_verifier.cpp
        include/h_file/security/file_hash_verifier.h

        include/cpp_file/security/log_viewer.cpp
        include/h_file/security/log_viewer.h

        include/cpp_file/security/advanced_security_menu.cpp
        include/h_file/security/advanced_security_menu.h

        include/cpp_file/file/clear_temp_file.cpp 
        include/h_file/file/clear_temp_file.h 
        
        include/cpp_file/activator/menu.cpp 
        include/h_file/activator/menu.h 

        include/cpp_file/activator/load_script.cpp 
        include/h_file/activator/load_script.h

        include/cpp_file/system_info/system_info.cpp
        include/h_file/system_info/system_info.h

        include/cpp_file/file/clear_cookie.cpp
        include/h_file/file/clear_cookie.h

        include/Logger.cpp
        include/Logger.h
)

target_link_libraries(7SCW 
    user32
    iphlpapi
    ws2_32
    advapi32
    wininet
)
@echo off
echo ========================================
echo    SIMPLE STATIC COMPILATION
echo ========================================
echo.

echo Checking for Visual Studio compiler...
where cl.exe >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Visual Studio found - compiling...
    echo.
    
    cl.exe /EHsc /MT /O2 /DNDEBUG /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING /std:c++17 7cpp.cpp terminal_commands.cpp /link /SUBSYSTEM:CONSOLE /MACHINE:X86 /OUT:7cpp_static.exe kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcrtd.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:oldnames.lib
    
    if %errorlevel% equ 0 (
        echo.
        echo [SUCCESS] Compilation completed!
        echo Static executable: 7cpp_static.exe
        dir 7cpp_static.exe | find "7cpp_static.exe"
    ) else (
        echo.
        echo [ERROR] Compilation failed!
    )
    goto :end
)

echo Checking for MinGW compiler...
where g++ >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] MinGW found - compiling...
    echo.
    
    g++ -std=c++17 -static -static-libgcc -static-libstdc++ -D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING -O2 -s -o 7cpp_static_mingw.exe 7cpp.cpp terminal_commands.cpp -lshlwapi -lole32 -luuid -loleaut32 -ladvapi32 -lws2_32 -liphlpapi
    
    if %errorlevel% equ 0 (
        echo.
        echo [SUCCESS] Compilation completed!
        echo Static executable: 7cpp_static_mingw.exe
        dir 7cpp_static_mingw.exe | find "7cpp_static_mingw.exe"
    ) else (
        echo.
        echo [ERROR] Compilation failed!
    )
    goto :end
)

echo [ERROR] No compilers found!
echo Please install Visual Studio or MinGW
echo.

:end
echo.
echo Press any key to exit...
pause >nul

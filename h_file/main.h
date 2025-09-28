#ifndef INC_7SCW_MAIN_H
#define INC_7SCW_MAIN_H

#include <windows.h>

// Declare external variable and function
extern volatile BOOL g_ctrlCPressed;
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

#endif // INC_7SCW_MAIN_H
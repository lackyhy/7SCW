#ifndef INC_7SCW_MAIN_H
#define INC_7SCW_MAIN_H

#include "windows.h"
#include "string"

const std::string VERSION = "9.9.5";

extern volatile BOOL g_ctrlCPressed;
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

#endif // INC_7SCW_MAIN_H

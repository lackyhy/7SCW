#ifndef INC_7SCW_MAIN_H
#define INC_7SCW_MAIN_H

#include "string"
#include "windows.h"

const std::string VERSION = "12.7";

extern volatile BOOL g_ctrlCPressed;
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

#endif // INC_7SCW_MAIN_H

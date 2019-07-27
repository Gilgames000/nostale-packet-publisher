#pragma once
#include <windows.h>

HANDLE ConnectPipe(LPCSTR szPipeName);
BOOL DisconnectPipe(HANDLE hPipe);
DWORD WriteToPipe(HANDLE hPipe, LPSTR szMessage);
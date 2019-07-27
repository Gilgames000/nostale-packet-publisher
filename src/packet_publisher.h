#pragma once
#include <windows.h>

BOOL FindAddresses();
BOOL StartPublisher();
BOOL StopPublisher();
BOOL HookSend();
BOOL HookRecv();
BOOL UnhookSend();
BOOL UnhookRecv();
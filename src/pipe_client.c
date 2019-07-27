#include <windows.h>
#include <stdio.h>
#include "pipe_client.h"

HANDLE ConnectPipe(LPCSTR szPipeName)
{
    HANDLE hPipe;

    for (;;)
    {
        printf("[INFO]: Trying to connect to the pipe server at '%s'... ", szPipeName);
        hPipe = CreateFileA(
            szPipeName,
            GENERIC_WRITE,
            0,    // no sharing
            NULL, // default security attributes
            OPEN_EXISTING,
            0,     // default attributes
            NULL); // no template file

        if (hPipe != INVALID_HANDLE_VALUE)
        {
            printf("SUCCESS\n");
            break;
        }
        printf("FAILURE\n");

        printf("[INFO]: The pipe server may be busy or offline; waiting for a server...\n");
        while (!WaitNamedPipeA(szPipeName, NMPWAIT_WAIT_FOREVER))
            Sleep(5 * 1000);

        printf("[INFO]: Pipe server found\n");
    }

    return hPipe;
}

BOOL DisconnectPipe(HANDLE hPipe)
{
    printf("[INFO]: Pipe server disconnected\n");
    return CloseHandle(hPipe);
}

DWORD WriteToPipe(HANDLE hPipe, LPSTR szMessage)
{
    BOOL bSuccess;
    DWORD dwDummy;

#ifdef NT_PUB_LOG
    printf("[PIPE]: %s", szMessage);
#endif

    bSuccess = WriteFile(
        hPipe,             // pipe handle
        szMessage,         // message
        strlen(szMessage), // message length
        &dwDummy,          // bytes written
        NULL);             // not overlapped

    return bSuccess || GetLastError() == ERROR_IO_PENDING;
}
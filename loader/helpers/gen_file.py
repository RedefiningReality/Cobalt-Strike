# generate test resource that would mimic payload with specific encoding
# will choose characters from characters string at random

import random

characters = "abcdefghijklmnopqrstuvwxyz"   # charset - currently all lowercase characters
length = 304155                             # file length
filename = "lowercase.ico"                  # output file name

with open(filename, 'wb') as file:
    for _ in range(length):
        char = random.choice(characters)
        file.write(char.encode('utf-8'))
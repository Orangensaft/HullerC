import os
import sys
blocks=10
bdir="bench"
try:
    os.stat(bdir)
except:
    os.mkdir(bdir)
tosplit = open(sys.argv[1])
inhalt = tosplit.readlines()
tosplit.close()
blen=len(inhalt)//blocks
for i in range(blocks):
    f=open("bench/"+str(i)+".svm","a")
    f.writelines(inhalt[i*blen:blen*(i+1)])
    f.close()


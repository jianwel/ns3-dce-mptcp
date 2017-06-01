#!/usr/bin/python
import csv
import glob
import sys
from sumUdp import getResult


if len(sys.argv) != 2:
    print "argc error"
nUE = int(sys.argv[1])

iperfl=[]
lossl=[]

for i in range(0, nUE):
    re = getResult(i)
    iperfl.append(re.th)
    lossl.append(re.loss)





#this shows how to get reading of user 1
for i in range(0, nUE):
    sys.stdout.write(str(iperfl[i]))
    sys.stdout.write(" ")
sys.stdout.write("\n")


for i in range(0, nUE):
    sys.stdout.write(str(lossl[i]))
    sys.stdout.write(" ")
sys.stdout.write("\n")

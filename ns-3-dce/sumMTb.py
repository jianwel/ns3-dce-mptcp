#!/usr/bin/python
import csv
import glob
import sys
from collections import defaultdict


if len(sys.argv) != 2:
    print "argc error"
nUE = int(sys.argv[1])

datal=[]

columns = defaultdict(list) # each value in each column is appended to a list
readName="DlTxPhyStats.txt"
with open(readName, 'r') as csvfile:
    reader = csv.DictReader(csvfile, delimiter='\t',
                                quotechar='|') # read rows into a dictionary format
    for row in reader: # read a row as {column1: value1, column2: value2,...}
        for (k,v) in row.items(): # go over each column name and value
           #print k+":"+v
            columns[k.strip()].append(v)
#idl = columns["RNTI"]
idl = columns["IMSI"]
#l = columns["size"]
l = columns["size"]
#print idl

#this shows how to get reading of user 1
for i in range(1, nUE+1):
    ll = [x for ind, x in enumerate(l) if idl[ind]==str(i)]
     #print ll
    if len(ll) != 0:
        avgl = sum(map(int, ll)) / float(len(ll))
        datal.append(avgl)
        print str(i)+" "+str(avgl)
    else:
        print str(i)+ " " + str(0)
for ii in datal:
    sys.stdout.write(str(ii))
    sys.stdout.write(" ")
sys.stdout.write("\n")

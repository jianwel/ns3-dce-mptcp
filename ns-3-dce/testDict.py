#!/usr/bin/python
import csv
import glob
import sys
from collections import defaultdict

columns = defaultdict(list) # each value in each column is appended to a list
readName="DlRxPhyStats.txt"
with open(readName, 'r') as csvfile:
    reader = csv.DictReader(csvfile, delimiter='\t',
                                quotechar='|') # read rows into a dictionary format
    for row in reader: # read a row as {column1: value1, column2: value2,...}
        for (k,v) in row.items(): # go over each column name and value
           #print k+":"+v
            columns[k.strip()].append(v)
idl = columns["IMSI"]
l = columns["size"]
#print idl

#this shows how to get reading of user 1
print [x for ind, x in enumerate(l) if idl[ind]==str(1)]
#print sum(map(int, l)) / len(l)

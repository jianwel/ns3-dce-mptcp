#!/usr/bin/python
import csv
import glob
import sys
readName="DlRxPhyStats.txt"
with open(readName, 'r') as csvfile:
        r = csv.reader(csvfile, delimiter='\t',
                                quotechar='|')
        for rr in r:
            print rr;

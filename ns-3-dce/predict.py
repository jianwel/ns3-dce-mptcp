#!/usr/bin/python
import csv
import numpy
import sys

if(len(sys.argv) ==0):
    folderName = "20160108-201754-lte-ok"
else:
    folderName = sys.argv[1]

print folderName
i=10
listR=[]
rates=[]

for i in range(1, 20):
    with open('data/'+folderName+'/iperfRe/'+str(i)+'.txt', 'r') as csvfile:
            r = csv.reader(csvfile, delimiter=' ',
                                    quotechar='|')
            row_count = sum(1 for row in r)
            #print row_count
            ii=0
            csvfile.seek(0);
            for rr in r:
                listR.append(int(rr[0]));
            #print len(listR)


            for ii in range(3, row_count):
            #for ii in range(3, 6):
                if(listR[ii] != 0):
                    pre = listR[ii-3]*0.09 + listR[ii-2]*0.21 + listR[ii-1]*0.7;
                    diff = abs(pre - listR[ii])
                    rate=  float(diff) / listR[ii]
                    rates.append(rate);
                    print "%d %d %f %d"%(i, ii, rate, listR[ii])

print numpy.mean(rates)
print numpy.std(rates)


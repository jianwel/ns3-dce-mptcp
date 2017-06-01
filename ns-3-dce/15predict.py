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

def threePerf(start, listR):
    return listR[start]+listR[start+1]+listR[start+2]

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


            for ii in range(6, row_count-2, 3):
            #for ii in range(3, 6):
                current = threePerf(ii, listR)
                if(current != 0):
                    pre = threePerf(ii-6, listR)*0.3 + threePerf(ii-3, listR)*0.7;
                    diff = abs(pre - current)
                    rate=  float(diff) / current
                    rates.append(rate);
                    print "%d %d %f %d"%(i, ii, rate, current)

print numpy.mean(rates)
print numpy.std(rates)


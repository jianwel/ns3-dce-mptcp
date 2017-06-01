#!/usr/bin/python
import csv
import numpy
import sys

print len(sys.argv)

if(len(sys.argv) <=1):
    folderName = "20160109-220239-lte-speed-0"
else:
    folderName = sys.argv[1]

print folderName
i=10

rates=[]

for i in range(1, 20):

    listR=[]
    iperfL=[]
    predictL=[]
    fitL=[]

    oneDiff=[]

    with open('data/'+folderName+'/iperfRe/'+str(i)+'.txt', 'r') as iperfF:
        r = csv.reader(iperfF, delimiter=' ',
                                    quotechar='|')
        ii = 0
        for rr in r:
            iperfL.append(int(rr[0]))
            ii = ii + 1;
        if (ii < 5):
            continue;
    print iperfL




#    with open('data/'+folderName+'/sinrPredict/s'+str(i)+'.txt', 'r') as csvfile:
#            r = csv.reader(csvfile, delimiter=' ',
#                                    quotechar='|')

    lines = open('data/'+folderName+'/sinrPredict/s'+str(i)+'.txt', 'r').readlines()
    ii = 0;
    for l in lines:
        listR = l.split("  ");
        if(len(listR) == 13):
            ii = ii+1
            predictL.append(float(listR[12]))
        if(ii ==5):
            break
    print predictL



    ratio = iperfL[1] / predictL[1];
    print "ratio=%f"%ratio
    fitL = [ ratio*x for x in predictL]
   # print fitL
    sub = [x - y for x, y in zip(fitL, iperfL)]
    #print sub
    oneDiff = [x/y for x, y in zip(numpy.absolute(sub), iperfL)]
    print oneDiff
    rates.extend(oneDiff)




print numpy.mean(rates)
print numpy.std(rates)


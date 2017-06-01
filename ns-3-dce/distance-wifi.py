#!/usr/bin/python
import csv
import numpy
import sys

#print len(sys.argv)
#
#if(len(sys.argv) <=1):
#    folderName = "20160109-220239-lte-speed-0"
#else:
#    folderName = sys.argv[1]
#
#print folderName
i=0

rates=[]
#nameList=["20160109-233919-lte-t100-s0", "20160110-002228-lte-t100-s0.57", "20160110-010528-lte-t100-s15", "20160110-092754-lte-t100-s30"]
nameList=["20160110-140704-wifi-r80-s0", "20160110-142346-wifi-r80-s0.57", "20160110-144053-wifi-r80-s15", "20160110-135704-wifi-r80-s30" ]


myfile = open("data/%d-wifi.csv"%i, 'w')
wr = csv.writer(myfile, quoting=csv.QUOTE_ALL)
myfileRate = open("data/%d-wifi-rate.csv"%i, 'w')
wrR = csv.writer(myfileRate, quoting=csv.QUOTE_ALL)
#for i in range(1, 20):
#for i in range(0, 1):
for folderName in nameList:

    listR=[]
    iperfL=[]
    predictL=[]
    fitL=[]

    oneDiff=[]



    notset = 0
    firstNoneZ =5

    with open('data/'+folderName+'/iperfRe/'+str(i)+'.txt', 'r') as iperfF:
        r = csv.reader(iperfF, delimiter=' ',
                                    quotechar='|')
        ii = 0
        for rr in r:
            iperfL.append(float(rr[0])/1000000)
            if(notset and int(rr[0])!=0):
                notset = 1
                firstNoneZ = ii

            ii = ii + 1;
        if (ii < 19):
            print 'data/'+folderName+'/iperfRe/'+str(i)+'.txt'
            continue;
    #print iperfL




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
        if(ii == 19):
            break
    #print predictL





   # print fitL
    sub = [x - y for x, y in zip(predictL, iperfL)]
    #print sub
    #absSub = numpy.absolute(sub)
    absSub = (sub)
    wr.writerow(absSub)

    divide = [x/y if y else 0 for x, y in zip(absSub, iperfL) ]
    wrR.writerow(divide)

    for ii in range(0, len(absSub)):
        if(iperfL[ii] !=0):
            rates.append( absSub[ii] / iperfL[ii]);





print numpy.mean(rates)
print numpy.std(rates)


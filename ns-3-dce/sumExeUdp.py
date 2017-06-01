#!/usr/bin/python
import csv
import glob
import sys
import collections
import os

def findLargest(folder):
    if os.path.isdir(folder):
        allF = glob.glob(folder+'*')
        allF.sort()
        return allF[-2]
    else:
        return "error"

#returns string for this one, because sinr can be so large, and will need to print it as scientific format
def getSINR():
    sinr = 0
    rsrp =0
    Result = collections.namedtuple('Result', ['sinr', 'rsrp'])
    if not os.path.exists('DlRsrpSinrStats.txt'):
        return Result(-1, -1)

    with open('DlRsrpSinrStats.txt', 'r') as csvfile:
        r = csv.reader(csvfile, delimiter='\t',
                                quotechar='|')
        nrows = 0;
        for rr in r:
            if nrows ==1:
                sinr = rr[-1]
                rsrp = rr[-2]
                return Result(sinr, rsrp)
            nrows+=1


def getMac():
    Result = collections.namedtuple('Result', ['mcs', 'tbSize'])
    if not os.path.exists('DlRxPhyStats.txt'):
        return Result(-1, -1)


    with open('DlRxPhyStats.txt', 'r') as csvfile:
        r = csv.reader(csvfile, delimiter='\t',
                                quotechar='|')
        nrows = 0;
        for rr in r:
            if nrows ==1:
                return Result(int(rr[6]), int(rr[7]))
            nrows+=1




def getResult():
    larger = findLargest('files-%d/var/log/'%0)
    Result = collections.namedtuple('Result', ['th', 'loss'])
    #print "larger="+str(larger)
    if larger=="error":
        print "get largest file error"
        return Result(-1, -1)

    sumT =0
    avgT = 0
    loss = 0

    with open(larger+'/stdout', 'r') as csvfile:
        r = csv.reader(csvfile, delimiter=',',
                                quotechar='|')
        nrows = 0;
        for rr in r:
            sumT = sumT + float(rr[8])/1000000.0
            nrows = nrows +1;
            if len(rr) >9:
                #print rr[-2]
                #print "loss="+rr[-2]
                #if rr[-2] == '0.000':
                loss=float(rr[-2])
        if(nrows != 0):
            avgT = sumT / (nrows)
            #print "%.4f"%(avgT)
        else:
            print "no valid records"
        re = Result(avgT, loss)
        return re

re = getResult()
re2 = getSINR()
reMac = getMac()

#aa = float(re2.sinr)
#bb = float(0.000)
#cc = float(1e10)


print "SINR\tMCS\tTransport Block Size\tGoodput\tLoss rate"
print "%s\t%d\t%d\t%.4f\t%.4f"%(re2.sinr, reMac.mcs, reMac.tbSize, re.th, re.loss)



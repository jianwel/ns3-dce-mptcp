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
    #sinr = 0
    #rsrp =0
    Result = collections.namedtuple('Result', ['sinr', 'rsrp'])
    if not os.path.exists('DlRsrpSinrStats.txt'):
        return Result(-1, -1)

    with open('DlRsrpSinrStats.txt', 'r') as csvfile:
        r = csv.reader(csvfile, delimiter='\t',
                                quotechar='|')
        nrows = 0;
        sumSinr = 0;
        sumRsrp = 0;
        for rr in r:
            if nrows!=0:
                sumSinr += float(rr[-1]);
                sumRsrp += float(rr[-2]);
            nrows+=1
        if nrows !=0:
            sinr = sumSinr / nrows;
            rsrp = sumRsrp / nrows
            return Result(sinr, rsrp)
        else:

            return Result(0, 0)


def getMac():
    Result = collections.namedtuple('Result', ['mcs', 'tbSize'])
    if not os.path.exists('DlRxPhyStats.txt'):
        return Result(-1, -1)


    with open('DlRxPhyStats.txt', 'r') as csvfile:
        r = csv.reader(csvfile, delimiter='\t',
                                quotechar='|')
        nrows = 0;
        sumMcs=0
        sumTb=0
        for rr in r:
            if nrows!=0:
                sumMcs+=int(rr[6])
                sumTb+=int(rr[7])
            nrows+=1
        if nrows !=0:
            return Result(sumMcs/nrows, sumTb/nrows)
        else:
            return Result(0, 0)





def getResult(ueID):
    larger = findLargest('files-%d/var/log/'%ueID)
    Result = collections.namedtuple('Result', ['th', 'loss'])
    #print "larger="+str(larger)
    if larger=="error":
        print "get largest file error"
        return Result(-1, -1)

    sumT =0
    avgT = 0
    loss = 0

    if ueID==0:
        with open('iperf_detailed.txt', 'w') as iperfD:
           pass

    with open(larger+'/stdout', 'r') as csvfile:
        r = csv.reader(csvfile, delimiter=',',
                                quotechar='|')
        nrows = 0;
        for rr in r:
            with open('iperf_detailed.txt', 'a+') as iperfD:
                iperfD.write(rr[8]+" ");
            sumT = sumT + float(rr[8])/1000000.0
            nrows = nrows +1;
            if len(rr) >9:
                loss=float(rr[-2])

        with open('iperf_detailed.txt', 'a+') as iperfD:
            iperfD.write("\n")
        if(nrows != 0):
            avgT = sumT / (nrows)
            #print "%.4f"%(avgT)
        else:
            print "no valid records"
        re = Result(avgT, loss)
        return re


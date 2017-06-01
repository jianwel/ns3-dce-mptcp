#!/usr/bin/python
import os
import sys
import time
import math
from tables import *
from sumUdp import *
#from sumUdp import getMac

timestr = time.strftime("%Y%m%d-%H%M%S")
folderName="data/newUdp-"+timestr
os.mkdir(folderName)

fb = 20000000
nUE = 1
#Dlist =[500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000]
Dlist =[0, 50, 100, 300, 500, 1000, 2000,  3000, 4000,  5000,  6000, 7000,  8000]

fre= open(folderName+"/allResults.txt", 'a+')
titleStr = "Number of UE, Distance, SINR, SINR(dB),  MCS, Number of Bits, Transport Block Size, UDP Goodput, TCP Goodput, Loss Rate, Shannon Capacitya, Predict Error, Predict Error Rate,  Nyquist Rate, MCS Predict, Predict based on TB size\n"

fre.write(titleStr)
fre.close()


#for all the runs, cp the default file, write variables fb into readme.txt
# in dataFolder, should also have allResults.txt

#inner folder should have pcap, result.txt

def runOnce(nUE, D, dataFolder, fb):
    if not os.path.exists(dataFolder+"/"+str(nUE)):
        os.mkdir(dataFolder+"/"+str(nUE))

    if not os.path.exists(dataFolder+"/"+str(nUE)+"/"+str(D)):
        os.mkdir(dataFolder+"/"+str(nUE)+"/"+str(D))

    else:
        print "alreay generated D="+str(D);
        return;

    innerFolder = dataFolder+"/"+str(nUE)+"/"+str(D)

    #if search direction changes, then step =/2

    #run once, and set to result +2m

    #for every D, we will create a folder, and the following will
    #be run several times in a for loop

    #put this into a function, which has two arguements, D, nUE

    #the one line result file will be written to the sub-folder and the top level result file D=0-8000km+nUE=1-8.txt

    #tryValue=18000000


    #0 is decreasing 1 increasing
    searchD = 0
    nRun=0
    #run tcp first
    outfileName = "stdout.txt"


    tcpStopTime = 60
    udpStopTime = 30


    runstr = "./waf --run \"oneNewLTE --nUE=%d "%nUE+ " --D="+str(D)+" --disWifi=true --isUDP=0 --stopTime=" + str(tcpStopTime)+" --ns3::ConfigStore::Filename=../../lte.default\"  | tee %s "%outfileName;
    print runstr
    os.system(runstr)
    result = getResult(0)
    tcpTh = result.th;
    print "tcpTh="+str(tcpTh)
    os.mkdir(innerFolder+"/tcp-pcap")
    os.mkdir(innerFolder+"/tcp")
    #copy files to folder
    os.system("mv *.pcap "+innerFolder+"/tcp-pcap")
    os.system("mv *.txt "+innerFolder+"/tcp")
    os.system("make clean")


    tryValue=tcpTh * 5000000;
    step = 4000000
    lossrate = 20



    print "tryValue"+str(tryValue);
    while True:

   # while not (lossrate >= 7 and lossrate <= 10):
        runstr = "./waf --run \"oneNewLTE --nUE=%d "%nUE+ " --D="+str(D)+" --B=%d"%tryValue+" --disWifi=true --stopTime="+str(udpStopTime)+" --ns3::ConfigStore::Filename=../../lte.default\"  | tee %s "%outfileName;
        print runstr
        os.system(runstr)
        result = getResult(0)
        print "get loss rate from result:"+str(result.loss) +"  "+str(result.th)
        nRun+=1;
        print "nRun="+str(nRun)

        lossrate = result.loss

        if (lossrate >= 7 and lossrate <= 10):
            break;

        if nRun ==1:
            tryValue = (result.th) * (1000000 * 0.9)
            continue

        if lossrate >10:
            if searchD == 1:
                step /=2
            tryValue -= step
            if tryValue<=0:
                tryValue=50
            searchD=0
        elif lossrate <7:
            if searchD == 0:
                step /=2
            tryValue += step
            searchD=1
        print "*****step="+str(step)
        print "*****try"+str(tryValue)
        print "*****searchD="+str(searchD);
        #break;


    print "nRun="+str(nRun)

    re = getResult(0)
    re2 = getSINR()
    reMac = getMac()


    #TODO: need to use the weighted average of #rb of every user to calc the scale here
    scale = 0.25;
    shannon = math.log(1+float(re2.sinr)) * (fb /1000000.0) * scale;
    nyquist = (2* fb/ 1000000.0) * math.log(getNBits(reMac.mcs), 2) *scale
    perror = shannon - float(tcpTh)
    perrorRate = perror / float(tcpTh)



    print titleStr

    #print "%d\t%d\t%s\t%d\t%d\t%d\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f"%(nUE, D, re2.sinr, reMac.mcs, getNBits(reMac.mcs), reMac.tbSize, re.th, tcpTh, re.loss, shannon, nyquist, 16.8*scale*getNBits(reMac.mcs), reMac.tbSize/1000.0 *8)

    #one item per row

    dataStr =  (str(nUE) + ",  " +
            str(D) + ",  " +
            re2.sinr + ",  " +
            str( math.log(float(re2.sinr), 10) ) + ",  " +
            str(reMac.mcs) + ",  " +
            str(getNBits(reMac.mcs)) + ",  " +
            str(reMac.tbSize) + ",  " +
            str(re.th) + ",  " +
            str(tcpTh) + ",  " +
            str(re.loss) + ",  " +
            str(shannon) + ",  " +
            str(perror) + ",  " +
            str(perrorRate) + ",  " +
            str(nyquist) + ",  " +
            str(16.8*scale*getNBits(reMac.mcs)) + ",  " +
            str(reMac.tbSize/1000.0 *8) + "\n" )
           # str() + ",  " +
    print dataStr

    os.mkdir(innerFolder+"/udp-pcap")
    os.mkdir(innerFolder+"/udp")
        #copy files to folder
    os.system("mv *.pcap "+innerFolder+"/udp-pcap")
    os.system("mv *.txt "+innerFolder+"/udp")

    fre= open(innerFolder+"/result.txt", 'w')
    fre.write(dataStr)

    fre.close()

    fre= open(dataFolder+"/allResults.txt", 'a')

    fre.write(dataStr)
    fre.close()


    fdes =open(innerFolder+"/readme.txt", 'w');
    fdes.write(runstr);
    fdes.write("***********************\n")
    fdes.write("frequecy bandwidth ="+str(fb))
    fdes.close()
    os.system("make clean")

for ii in Dlist:
    runOnce(1, ii, folderName, fb)

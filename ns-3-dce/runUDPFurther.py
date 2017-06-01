#!/usr/bin/python
import os
import sys
import time
import math
from tables import *
from sumUdp import *
#from sumUdp import getMac

timestr = time.strftime("%Y%m%d-%H%M%S")
folderName="data/newUdpOnlyMulti-"+timestr
os.mkdir(folderName)


fb = 20000000
#nUE = 1
nUEList = [1, 4, 8, 16]
Dlist =[8000]
#nRandoms=1
#Dlist =[1300, 1500, 9000, 10000, 11000, 12000]
tcpR = {1: 4.51,
        4: 1.12,
        8: 0.56,
        16: 0.2801
        }


titleStr = "Number of UE, Distance, SINR, SINR(dB),  MCS, Number of Bits, Transport Block Size, UDP Goodput, TCP Goodput, Loss Rate, Shannon Capacity, Predict Error, Predict Error Rate,  Nyquist Rate, MCS Predict, Predict based on TB size\n"



#for all the runs, cp the default file, write variables fb into readme.txt
# in dataFolder, should also have allResults.txt

#inner folder should have pcap, result.txt

def runOnce(nUE, D, dataFolder, fb, isFurther):
    if not os.path.exists(dataFolder+"/"+str(nUE)):
        os.mkdir(dataFolder+"/"+str(nUE))

    if not os.path.exists(dataFolder+"/"+str(nUE)+"/"+str(D)):
        os.mkdir(dataFolder+"/"+str(nUE)+"/"+str(D))

    else:
        print "alreay generated D="+str(D);
       # return;

    innerFolder = dataFolder+"/"+str(nUE)+"/"+str(D);



    if nUE==1:
          with open(dataFolder+"/allResults.txt", 'w') as fre:
            fre.write(titleStr)
            fre.close()


    #if search direction changes, then step =/2

    #run once, and set to result +2m

    #for every D, we will create a folder, and the following will
    #be run several times in a for loop

    #put this into a function, which has two arguements, D, nUE

    #the one line result file will be written to the sub-folder and the top level result file D=0-8000km+nUE=1-8.txt

    #tryValue=18000000


    #0 is decreasing 1 increasing
    #run tcp first
    outfileName = "stdout.txt"


    tcpStopTime = 60
    udpStopTime = 30
    #randomString = 'NS_GLOBAL_VALUE="RngRun="' + str(runI) + " ";
    randomString =  "";

    print randomString

    runstr = randomString +"./waf --run \"oneNewLTE --nUE=%d "%nUE+ " --D="+str(D)+ " --isOneFurther=%d"%isFurther + " --B=%d"%(tcpR[nUE]*1000000) +" --disWifi=true --isUDP=0  --stopTime=" + str(udpStopTime)+" --ns3::ConfigStore::Filename=../../lte.default\"  | tee %s "%outfileName;
    print runstr
    os.system(runstr)
    result = getResult(0)
    Th = result.th
    print "UDP Th="+str(Th)
    loss= result.loss
    print "UDP los="+str(loss)

    re2 = getSINR()
    reMac = getMac()


    #TODO: need to use the weighted average of #rb of every user to calc the scale here
    scale = 0.25;
    shannon = math.log(1+float(re2.sinr), 2) * (fb /1000000.0) * scale;
    nyquist = (2* fb/ 1000000.0) * math.log(getNBits(reMac.mcs), 2) *scale


    perror = shannon - float(Th)
    perrorRate = perror / float(Th) *100.0


    print titleStr



    #one item per row

    dataStr =  (str(nUE) + ",  " +
            str(D) + ",  " +
            re2.sinr + ",  " +
            str( math.log(float(re2.sinr), 10) ) + ",  " +
            str(reMac.mcs) + ",  " +
            str(getNBits(reMac.mcs)) + ",  " +
            str(reMac.tbSize) + ",  " +
            str(Th) + ",  " +
            str(0) + ",  " +
            str(loss) + ",  " +
            str(shannon) + ",  " +
            str(perror) + ",  " +
            str(perrorRate) + ",  " +
            str(nyquist) + ",  " +
            str(16.8*scale*getNBits(reMac.mcs)) + ",  " +
            #str(reMac.tbSize/1000.0 *8) +",  " +
            str(reMac.tbSize/1000.0 *8) +"\n")
           #str(peRate) + "\n" )
           # str() + ",  " +
    print dataStr



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



    #copy files to folder
    os.system("mv *.txt "+innerFolder)
    os.system("make clean")



isFurther=1

if isFurther==1:
    FfolderName = folderName +"/further"
else:
    FfolderName = folderName +"/no-further"

os.mkdir(FfolderName)

for ii in nUEList:
    runOnce(ii, 8000, FfolderName, fb, isFurther)

isFuther=0

if isFurther==1:
    FfolderName = folderName +"/further"
else:
    FfolderName = folderName +"/no-further"

os.mkdir(FfolderName)

for ii in nUElist:
    runOnce(ii, 8000, FfolderName, fb, isFurther)





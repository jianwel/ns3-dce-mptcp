#!/usr/bin/python
import os
import sys
import time

timestr = time.strftime("%Y%m%d-%H%M%S")
os.mkdir("data/"+timestr);
speeds=[30, 0, 0.57, 15]

speedFlag="--globalSpeed=0"

if(len(sys.argv)!=0):
    speedFlag= "--globalSpeed=%f"%speeds[int(sys.argv[1])]

print speedFlag

#cp the default file


runstr = "./waf --run \"dce-mptcp-l --nUE=20 "+ speedFlag+" --disWifi=true --stopTime=100 --ns3::ConfigStore::Filename=../../lte.default\" ";
#runstr = "./waf --run \"dce-mptcp-l --nUE=20 --radius=150 --disLte=true --stopTime=30 --ns3::ConfigStore::Filename=../../lte.default\" ";
#runstr = "./waf --run \"dce-mptcp-l --nUE=20 --radius=150 --stopTime=30 --ns3::ConfigStore::Filename=../../lte.default\" ";

os.system(runstr);

os.system("./sum.py 20");

os.chdir("data/"+timestr);

os.mkdir("files")

os.system("cp ../../DlRsrpSinrStats.txt .");
os.system("cp ../../output-attributes.txt .");
os.system("cp ../../DlMacStats.txt .");

os.system("cp -ra ../../data/sinrPredict .");


os.system("cp -ra ../../data/iperfRe .");

fdes =open("readme.txt", 'w');
fdes.write(runstr);

os.chdir("files")
os.system("cp -ra ../../../files-* .")

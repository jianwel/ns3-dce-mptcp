#!/usr/bin/python
import os
import sys
import time

timestr = time.strftime("%Y%m%d-%H%M%S")
os.mkdir("data/udp-"+timestr);

dis = 8000
div = 5
nUE = 1

def read

#cp the default file

for i in range(0, div+1):
    print "D="+str((dis/div)*i)
    runstr = "./waf --run \"new --nUE=%d "%nUE+ "--D=%f"%(dis/div)*i +" --disWifi=true --stopTime=20 --ns3::ConfigStore::Filename=../../lte.default\" ";
    os.mkdir()
    #copy files to folder
#runstr = "./waf --run \"dce-mptcp-l --nUE=20 --radius=150 --disLte=true --stopTime=30 --ns3::ConfigStore::Filename=../../lte.default\" ";
#runstr = "./waf --run \"dce-mptcp-l --nUE=20 --radius=150 --stopTime=30 --ns3::ConfigStore::Filename=../../lte.default\" ";

os.system(runstr);

os.system("./sum.py 1");

os.chdir("data/"+timestr);

os.mkdir("files")

os.system("cp ../../DlRsrpSinrStats.txt .");
os.system("cp ../../output-attributes.txt .");
os.system("cp ../../DlMacStats.txt .");

os.system("cp -ra ../../data/sinrPredict .");


os.system("cp -ra ../../data/iperfRe .");

fdes =open("readme.txt", 'w');
fdes.write(runstr);
fdes.write("***********************\n")
fdes.write("overall distance ="+str(dis))
fdes.write("div = "+str(div))

fdes.close()

os.chdir("files")
os.system("cp -ra ../../../files-* .")

#!/usr/bin/python
import os
import time

timestr = time.strftime("%Y%m%d-%H%M%S")
os.mkdir("data/"+timestr);


#cp the default file


runstr = "./waf --run \"dce-mptcp-l --nUE=3 --disWifi=true --stopTime=10 --ns3::ConfigStore::Filename=../../lte.defaulti\" ";

#os.system(runstr);

os.system("./sum.py 20");

os.chdir("data/"+timestr);

os.system("cp ../../DlRsrpSinrStats.txt .");
os.system("cp ../../output-attributes.txt .");

os.system("cp -ra ../../data/sinrPredict .");


os.system("cp -ra ../../data/iperfRe .");

fdes =open("readme.txt", 'w');
fdes.write(runstr);

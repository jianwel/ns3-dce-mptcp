import os


N = [8]
#N = [8, 16, 32]

for i in N:
    os.system("./runMultiUETcp.sh "+str(i))
    os.system("./sum.py " + str(i)  +" > iperfR.txt")
    os.system("mkdir data/multi/"+str(i))
    os.system("mv *.txt data/multi/"+str(i))
    os.system("make clean")

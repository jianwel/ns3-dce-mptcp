#!/usr/bin/python
import csv
import glob
import sys
def findLargest(folder):
    #allF = glob.glob(folder).sort()
    allF = glob.glob(folder+'*')
    allF.sort()
    #print allF
    return allF[-2]

nodes=int(sys.argv[1])
throughSum=0;

ff = open('data/iperfRe/avgs.txt', 'w');
tlist=[]

for i in range(0,nodes):
    larger = findLargest('files-%d/var/log/'%i)
    print "larger="+str(larger)
    #with open('files-0/var/log/46638/stdout', 'r') as csvfile:
    with open(larger+'/stdout', 'r') as csvfile:
        r = csv.reader(csvfile, delimiter=',',
                                quotechar='|')
        sum =0;
        nrows = 0;
        f = open('data/iperfRe/%d.txt'%i, 'w');
        for rr in r:
            sum = sum + float(rr[8]) / 1000000
            f.write(rr[8]+"\n");
            nrows = nrows +1;
            print rr[8]
        if(nrows != 0):
            avg = sum / (nrows)
            tlist.append(avg)
            #print "nodes %d, avg = %.4f"%(i, avg)
            print "%d  %.4f"%(i, avg)
            ff.write("%d  %.4f\n"%(i, avg))
            throughSum += avg;
        else:
            print "%d N/A"%(i)
            #print "%d  %.4f"%(i, avg)
ff.close()


print "%d %.4f"%(nodes, throughSum / nodes)
f = open('data/iperfRe/overallAvg.txt','w')
f.write("%d %.4f\n"%(nodes, throughSum / nodes))
f.close();
f = open('data/iperfRe/overallSum.txt','w')
f.write("%d %.4f\n"%(nodes, throughSum))
f.close();

for ii in tlist:
    sys.stdout.write(str(ii))
    sys.stdout.write(" ")
sys.stdout.write("\n")

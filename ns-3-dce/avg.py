import csv
#with open('files-0/var/log/46638/stdout', 'r') as csvfile:
with open('files-0/var/log/46643/stdout', 'r') as csvfile:
    r = csv.reader(csvfile, delimiter=',',
                            quotechar='|')
    sum =0;
    nrows = 0;
    for rr in r:
        sum = sum + float(rr[8]);
        nrows = nrows +1;
        #print rr[8]
    avg = sum / (nrows * 1000000)
    print "avg = %.4f"%avg

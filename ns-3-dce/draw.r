daq = read.table(file('mydata.dat'))
#hist(housing$Home.Value)
X11()
pairs(daq)

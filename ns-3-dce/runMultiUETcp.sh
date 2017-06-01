./waf --run "oneNewLTE --nUE=$1  --D=8000 --isUDP=0 --disWifi=true --stopTime=60 --ns3::ConfigStore::Filename=../../lte.default"  | tee stdout.txt

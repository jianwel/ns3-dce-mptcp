#!/bin/bash

#. ./utils/setenv.sh
#VERBOSE=""
STOPTIME=80

echo -n "Cooja (non-vdl) + Pthread:  "
NS_ATTRIBUTE_DEFAULT='ns3::TaskManager::FiberManagerType=PthreadFiberManager' time ./build/bin/dce-quagga-ospfd-rocketfuel --stopTime=${STOPTIME}  |grep -v Unsupported|grep -v bytes
echo -n "Cooja (non-vdl) + Ucontext: "
NS_ATTRIBUTE_DEFAULT='ns3::TaskManager::FiberManagerType=UcontextFiberManager' time ./build/bin/dce-quagga-ospfd-rocketfuel  --stopTime=${STOPTIME} |grep -v Unsupported|grep -v bytes

echo -n "Dlm (vdl) + Pthread:        "
NS_ATTRIBUTE_DEFAULT='ns3::DceManagerHelper::LoaderFactory=ns3::DlmLoaderFactory[];ns3::TaskManager::FiberManagerType=PthreadFiberManager' time ../build/bin/dce-runner ./build/bin/dce-quagga-ospfd-rocketfuel --stopTime=${STOPTIME}  |grep -v Unsupported|grep -v bytes
echo -n "Dlm (vdl) + Ucontext:       "
NS_ATTRIBUTE_DEFAULT='ns3::DceManagerHelper::LoaderFactory=ns3::DlmLoaderFactory[];ns3::TaskManager::FiberManagerType=UcontextFiberManager' time ../build/bin/dce-runner ./build/bin/dce-quagga-ospfd-rocketfuel --stopTime=${STOPTIME}  |grep -v Unsupported |grep -v bytes

#!/bin/bash
source /opt/intel/oneapi/setvars.sh > /dev/null 2>&1
/bin/echo "##" $(whoami) is compiling DPCPP_Essentials Module1 -- oneAPI Intro sample - 1 of 1 simple.cpp
export OverrideDefaultFP64Settings=1 
export IGC_EnableDPEmulation=1 
dpcpp simple.cpp ../lodepng.cpp
if [ $? -eq 0 ]; then ./a.out; fi


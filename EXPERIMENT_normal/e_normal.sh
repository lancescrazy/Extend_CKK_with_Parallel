#!/bin/bash
rm normal_output.txt
c++ -o normal normal_ckk.cpp -lpthread
for((i=65;i<100;i+=10));
do
./normal $i
done

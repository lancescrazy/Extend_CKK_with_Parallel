#!/bin/bash
rm parallel_output.txt
c++ -o parallel parallel_ckk.cpp -lpthread -lstdc++
for((i=70;i<170;i+=10));
do
./parallel $i
done

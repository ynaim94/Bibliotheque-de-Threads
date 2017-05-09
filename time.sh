#!/bin/bash

if [[ $# -ne 1 ]]
then
    echo "Usage : ./fibonacci i avec i entier"
    exit;
fi

if [ -f time.txt ]
then
    rm time.txt
fi

for i in `seq 1 $1`
do

    ./create-many $i  > tmp.txt
    TIME=`cat tmp.txt | grep time | awk '{print $3}' `
    echo "$i $TIME" >> time.txt
done

#gnuplot graph.conf

#!/bin/bash

#pas d'exécutable fibonacci ?
#additionner user + sys

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

    { time ./fibonacci $i 2> fibonacci.stderr ; } 2> tmp.txt
    TIME=`cat tmp.txt | grep real | awk '{print $2}' | sed 's/[a-z]*//g' | cut -c 2-`
    echo "$i $TIME" >> time.txt
done

#gnuplot graph.conf

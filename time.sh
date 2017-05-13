#!/bin/bash

#pas d'exécutable fibonacci ?
#additionner user + sys

if [[ $# -ne 2 ]]
then
    echo "Usage : ./time.sh nom_test valeur"
    exit;
fi

if [ -f time.txt ]
then
    rm time.txt
fi

for i in `seq 1 $2`
do

    ./$1 $i > tmp.txt
    TIME=`cat tmp.txt | grep time | awk '{print $3}'`
    echo "$i $TIME" >> time.txt
done

#gnuplot graph.conf

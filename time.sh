#!/bin/bash

#pas d'exécutable fibonacci ?
#additionner user + sys

if [[ $# -ne 2 ]]
then
    echo "Usage : ./time.sh nom_test valeur"
    exit;
fi

if [ -f time-$1.txt ]
then
    rm time-$1.txt
fi

for i in `seq 1 $2`
do
	
	taskset -c 1 ./$1 $i $i*2 > tmp.txt
	TIME=`cat tmp.txt | grep time | awk '{print $3}'`
	taskset -c 1 ./$1-pthread $i $i*2 > tmp.txt
		TIME2=`cat tmp.txt | grep time | awk '{print $3}'`
		echo "$i $TIME $TIME2" >> time-$1.txt
done


#gnuplot graph.conf

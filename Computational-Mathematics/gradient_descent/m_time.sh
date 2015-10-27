#!/bin/bash
for i in `ls ../input_matrix`
do
if [[ $i == *10.txt ]];
then
	echo $i :
	time ./graddes < ../input_matrix/$i 
	echo --------------
fi
done

echo -e 

for i in `ls ../input_matrix`
do
if [[ $i == *100.txt ]];
then
	echo $i :
	time ./graddes < ../input_matrix/$i
	echo --------------
fi

done

echo -e

for i in `ls ../input_matrix`
do
if [[ $i == *1000.txt ]];
then
	echo $i :
	time ./graddes < ../input_matrix/$i
	echo --------------
fi

done


make

for i in 1 2 4; do 
	echo Threads: $i
	time ./a.out 0 100  100000000 $i;
done

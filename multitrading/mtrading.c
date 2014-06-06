/*
 * mtrading.c
 *
 *  Created on: 30 апр. 2014 г.
 *      Author: alex
 */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <errno.h>


typedef struct my_thread_t{
	pthread_t thr;
	int i;
} my_thread_t;

int A, B, Nthr, Nparts;
double sums[100];

double f(double x){
	return sin(x)*cos(x*x);
}

void* Simpson_integral(void* args){
	int shift = *((int*)args);

	double step = ((double)B - (double)A) / Nparts;
	double a = (double)A + shift*step;
	double b = (double)A + (shift+1)*step;
	double sum = 0;
	while( a < B ){
		sum += (b-a)/6*(f(a)+4*(f((a+b)/2))+f(b));
		a += Nthr*step;
		b += Nthr*step;
	}
	sums[shift] = sum;
	return NULL;
}

void* Simpson_integral2(void* args){
	int shift = *((int*)args);

	double step = ((double)B - (double)A) / Nparts;
	double a = (double)A + shift*((double)B - (double)A) / Nthr;
	double b = a + step;
	double BB = (double)A + (shift+1)*((double)B - (double)A) / Nthr;
	double sum = 0;
	while( a < BB ){
		sum += (b-a)/6*(f(a)+4*(f((a+b)/2))+f(b));
		a += step;
		b += step;
	}
	sums[shift] = sum;
	return NULL;
}

int main(int argc, char* argv[]){
	my_thread_t thr[100];

	if (argc < 5){
		printf("Usage: program a b parts threads\n");
		return 0;
	}
	A = atoi(argv[1]);
	B = atoi(argv[2]);
	Nparts = atoi(argv[3]);
	Nthr = atoi(argv[4]);
	if (Nthr > 100){
		printf("# Number of threads mustn't be more than 100!\n");
		printf("# Running threads: 100\n...\n");
		Nthr = 100;
	}
	if (sums == NULL){
		perror("Allocation memory for sums");
		return 1;
	}

	//clock_t start, end;
	//clock_t run_time = 0;

	//run_time = clock();

	for (int i = 0; i < Nthr; i++){
		thr[i].i = i;
		pthread_create(&thr[i].thr, NULL, Simpson_integral2, &thr[i].i);
	}
	for (int i = 0; i < Nthr; i++)
			pthread_join(thr[i].thr, NULL);

	//run_time = clock() - run_time;

	double result = 0;
	for (int i = 0; i < Nthr; i++)
		result += sums[i];

	printf("Integral = %f\n", result);
	//printf("Time = %f\n", ((double)run_time)/(double)CLOCKS_PER_SEC);
	return 0;
}

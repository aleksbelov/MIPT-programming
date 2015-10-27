#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
struct p_args{
    int i, n;
    double res;
};

double f(double x){
	return 1/x;
}

#define N 20000000

void* calculate(void* args){    
    int h = (*(struct p_args *)args).n;
    int t = (*(struct p_args *)args).i;
    double sum = 0;
	t = t - h;
	while (t + h < N){
		sum += f(t+h+1);
		t += h;
	}
    (*(struct p_args *)args).res = sum;
	
    return NULL;
}


int main(int argc, char **argv){
	
	if (argc != 2){
		printf("Please, specify the number of threads\n");
		return 0;
	}
	int n = atoi(argv[1]);
	double sum = 0;
    
    pthread_t* threads = (pthread_t*) calloc(n, sizeof(pthread_t));
    struct p_args *args = (struct p_args *) calloc(n, sizeof(struct p_args));
	
    for (int i = 0; i < n; i++){
        args[i].i = i;
        args[i].n = n;
        pthread_create(&threads[i], NULL, calculate, (void*) &args[i]);

    }
    
    for (int i = 0; i < n; i++){
        pthread_join(threads[i], NULL);
        sum += args[i].res;
    }
    
	printf("Result: %f\n", sum);
	return 0;
}

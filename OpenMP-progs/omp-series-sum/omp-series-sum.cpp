#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

double f(int n){
    if (n == 0)
        return 0;
    return 1.0/(n);
}

int main(int argc, char **argv){
    int Nthreads = 5;
    int N = atoi(argv[1]);
    double *sum = (double*) calloc(N, sizeof(double));
    #pragma omp parallel num_threads(Nthreads) shared(sum)
    {
        int id = omp_get_thread_num();
        #pragma omp for schedule(auto)
        for (int i = 0; i < N; i++){
            sum[id] += f(i);
        }
    }
    for (int i = 1; i < N; i++)
        sum[0] += sum[i];

    printf("%.10f\n", sum[0]);
}

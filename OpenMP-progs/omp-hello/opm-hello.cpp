#include <stdio.h>
#include <omp.h>


void ordered_output_1(int N){
    int order = 0;
    #pragma omp parallel shared(order) num_threads(N)
    {
        printf("*");
        #pragma omp barrier
        #pragma omp single
            {printf("\n");}

        int id = omp_get_thread_num();
        while (order < N){
            if (order == id){
                printf("Hello, I`m %i\n", id);
                fflush(stdout);
                order++;
            }
        }
    }
}

// This modification works faster
void ordered_output_2(int N){
    #pragma omp parallel num_threads(N)
    {
        printf("*");
        #pragma omp barrier
        #pragma omp single
            {printf("\n");}

        int id = omp_get_thread_num();
        #pragma omp for ordered
        for (int i = 0; i < N; i++){
            #pragma omp ordered
            {
                printf("Hello, I`m %i\n", id);
            }
        }
    }
}

int main(){
    ordered_output_2(24);

}

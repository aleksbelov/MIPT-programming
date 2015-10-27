#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

double f(double x){
	return 1/x;
}

int main(int argc, char **argv){
	
	int rank = 0, size = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	if (argc != 2){
		if (rank == 0){
			printf("Usage: mpirun -np [N] %s [n]\n", argv[0]);
			printf("\tN - count of threads\n");
			printf("\tn - count of terms of the series to sum\n");
		}
		MPI_Finalize();
		return 0;
	}
	int n = atoi(argv[1]);
	int h = size;
	int t = rank;
	double sum = 0;
	t = t - h;
	while (t + h < n){
		sum += f(t+h+1);
		t += h;
	}
	
	
	double buf = 0;
	if (rank == 0){
		if (rank + 1 < size)
                	MPI_Send(&sum, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
        }
        else {
                MPI_Status status;
                MPI_Recv(&buf, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &status);
                sum += buf;
                if (rank < size -1)
                        MPI_Send(&sum, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
        }

	if(rank == size - 1)
		printf("Result: %f\n", sum);
	MPI_Finalize();
	return 0;
}

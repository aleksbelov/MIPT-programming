#include<stdio.h>
#include "mpi.h"

int main(int argc, char **argv){
	
	int rank = 0, size = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int number = 0;
	if (rank == 0){
		printf("I am %d of %d and my number is %d, send %d\n", rank, size, number, number+1);
		number++;
		MPI_Send(&number, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
	}
	else {
		MPI_Status status;
		MPI_Recv(&number, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
		printf("I am %d of %d and my number is %d, send %d\n", rank, size, number, number+1);
		number++;
		if (rank < size -1)
			MPI_Send(&number, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return 0;

}

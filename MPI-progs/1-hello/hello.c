#include<stdio.h>
#include"mpi.h"
int main(int argc, char **argv){
	int rank = 0, size = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	printf("Hello, World! I am %d of %d\n", rank, size);
	MPI_Finalize();
	return 0;
}

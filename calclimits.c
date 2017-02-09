/*! \file
	\brief This file contains calclimits function
	
	
*/

#include <stdio.h>
#include <stdlib.h>


/*! \struct limits_t
	\brief A structure containing min and max indexes used in for cycle
	
	Describes interval: [min, max)
	Usage:
	\code{.c}
	struct limits_t limits;
	...
	for (int i = limits.min; i < limits.max; i++){
		...
	}
	\endcode
*/
struct limits_t{
	int min, /*!< Left index. Will be included in cycle*/
		max; /*!< Right index. Will not be included in cycle*/
};


/*! \fn void calclimits(int Nx, int size, int rank, struct limits_t* limits)
	\brief Calculates indexes for balances load distribution between threads processing an array

	\param[in] Nx		Count of elements to be distributed
	\param[in] size		Count of threads
	\param[in] rank		Own thread number
	\param[out] limits	A structure containing boundary indexes for the thread
	
	
	Usage:
	\code{.c}
	struct limits_t limits;
	...
	calclimits(Nx, size, rank, &limits);
	for (int i = limits.min; i < limits.max; i++){
		...
	}
	\endcode
*/
void calclimits(int Nx, int size, int rank, struct limits_t* limits){
	limits->min = limits->max = 0;
	
	limits->min = Nx / size * rank;

	if (rank == size - 1)
		limits->max = Nx;
	else
		limits->max = (Nx / size) * (rank + 1);
	int r = size - 1;
	for (int i = (Nx % size - 1); i > 0; i--){
		if (rank >= r)
			limits->min++;
		if (rank >= r-1 && rank != size - 1)
			limits->max++;
			r--;
	}
	return;
}

/*!
	\brief Main function. Contains a demonstration test for calclimits function

*/

int main(int argc, char** argv){
	int Nx = 18, size = 4;
	printf("Nx = %d\nsize = %d\n", Nx, size);
	for(int rank = 0; rank < size; rank++){
		struct limits_t limits;
		calclimits(Nx, size, rank, &limits);
		printf("%d: [%d, %d). %d elements\n", rank, limits.min, limits.max, limits.max - limits.min);
		
	}
	
	return 0;
}

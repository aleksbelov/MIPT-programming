#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>


void* new_pth(void*)
{
	printf("I am a thread!!! :)\n");
	return NULL;
}



int getnum(int argc, char* argv[])
{
	char *endptr = NULL;
        if (argc < 2)
        {
                printf("Please, try to run this program and enter a NUMBER again!\n");
                return -1;
        }
        int N = strtol(argv[1], &endptr, 0);

        if (*endptr != '\0')
        {
                printf("Please, try to run this program and enter a NUMBER again!\n");
               	return -1;
        }
	
	return N;
}


int main (int argc, char* argv[])
{
	int num = getnum(argc, argv);
	
	for (int i = 0; i < num; i++)
	{
		pthread_t id;
		void *(*start)(void*) = new_pth;
		pthread_create(&id, NULL, start, NULL);
		//if (pid == 0)
		{
		//	printf("I am a \n");
		//	return 0;
		}
		
		void* status = 0;
		pthread_join(id, &status);
	}

	return 0;
}

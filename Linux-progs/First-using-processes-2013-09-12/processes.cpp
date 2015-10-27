#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


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
	char *endptr = NULL;
	if (argc < 2)
	{
		printf("Please, try to run this program and enter a NUMBER again!\n");
		return 0;
	}
	int num = strtol(argv[1], &endptr, 0);
	
	if (*endptr != '\0')
	{
		printf("Please, try to run this program and enter a NUMBER again!\n");
		return 0;
	}
	

	for (int i = 0; i < num; i++)
	{
		
		int pid = fork();
		if (pid == 0)
		{
			printf("I am a %i child. My PID is %i\n", i, getpid());
			return 0;
		}
		int status;
		wait(&status);
	}

	return 0;
}

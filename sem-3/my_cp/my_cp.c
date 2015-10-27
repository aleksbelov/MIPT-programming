#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main (int argc, char* argv[])
{
	if (argc != 3) 
	{
		printf ("Need 2 arguments...\n");
		exit(EXIT_FAILURE);
	}
	FILE* source = fopen(argv[1], "r+");
	if (!(source))
	{
		printf("No such source file...\n");
		exit(EXIT_FAILURE);
	}
	FILE* copy = fopen(argv[2], "a");
	if (!(source && copy)) printf("Error opening file!\n");
	char buffer;
	printf("%p %p\n", source, copy);
	
	while ((buffer = getc(source)) != EOF)
		putc(buffer, copy);
	fclose(source);
	fclose(copy);
	return 0;
}

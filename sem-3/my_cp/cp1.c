#include <stdio.h>

int main (int argc, char* argv[])
{
	if (argc != 3) 
	{
		printf ("Need 2 arguments...\n");
		return 0;
	}
	FILE* source = fopen(argv[1], "r+");
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
#include <stdio.h>

int main (int argc, char* argv[])
{
	if (argc != 3) 
	{
		printf ("Need 2 arguments...\n");
		return 0;
	}
	FILE* source = fopen(argv[1], "r+");
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

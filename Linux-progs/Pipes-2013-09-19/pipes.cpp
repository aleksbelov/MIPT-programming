#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char* argv[])
{
	int pipefd[2];
	pid_t pid;
	
	if (argc < 2) {
		printf("Need file!\n");
		exit(EXIT_FAILURE);
	}
	
	if (pipe(pipefd) == -1) {
		printf("Pipe error!\n");
		exit(EXIT_FAILURE);
	}

	pid = fork();

	if (pid == -1) {
		printf("Fork error!\n");
		exit(EXIT_FAILURE);
	}
	
	int buf_size = 4;

	if (pid == 0){
		close(pipefd[0]);

		int real_size = 0;
		char buf[buf_size];

		int fd_sourse = open(argv[1], O_RDONLY);
		
		while ( (real_size = read(fd_sourse, buf, buf_size) ) > 0 )
			write(pipefd[1], buf, real_size);
		if (real_size < 0)
			exit(EXIT_FAILURE);
		close(fd_sourse);
		close(pipefd[1]);
	}
	else {
		close(pipefd[1]);
		
		int real_size = 0;
		char buf[buf_size];
		
		while ( (real_size = read(pipefd[0], buf, buf_size) ) > 0 )
			write(STDOUT_FILENO, buf, real_size);
		
		if (real_size < 0)
                        exit(EXIT_FAILURE);
		close(pipefd[0]);
	}
	
	return 0;
}

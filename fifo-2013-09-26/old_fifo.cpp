#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

const char* FIFO_NAME = "Fifo_file";
const char* FIFO_OUT_NAME = "out.fifo";
const char* FIFO_CONTROL_NAME = "Control_fifo";
const int BUF_SIZE = 102400;

int reader_from_fifo(){
	int fd_fifo = open(FIFO_NAME, O_RDONLY);
	//perror("open");
	int fd_out = open(FIFO_OUT_NAME, O_WRONLY | O_CREAT, 0664);
	char buf[BUF_SIZE];
	int real_size = 0;
	while ((real_size = read(fd_fifo, buf, BUF_SIZE)) > 0)
		write(fd_out, buf, real_size);

	if (real_size < 0)
		exit(EXIT_FAILURE);
	
	close(fd_out);
        close(fd_fifo);

	return 0;
}


int main(int argc, char* argv[]){

	if (argc == 1) {
		reader_from_fifo();
		exit(0);
	}

	if (argc > 2 )
		printf("Unnecessary arguments. "
			   "I use the first one only as src file\n");
	// main() = writer_to_fifo()
	
	
	
	int fd_src = open(argv[1], O_RDONLY);
	if (fd_src == -1){
		perror("Open");
		exit(EXIT_FAILURE);
	}

	if (mkfifo(FIFO_CONTROL_NAME, 0664) < 0){
                perror("CONTROL_FIFO");
                exit(EXIT_FAILURE);
		//return 1;
        }


	mkfifo(FIFO_NAME, 0664);
	int fd_fifo = open(FIFO_NAME, O_WRONLY);
	
	
	char buf[BUF_SIZE];
	int real_size = 0;
	while ((real_size = read(fd_src, buf, BUF_SIZE)) > 0)
		write(fd_fifo, buf, real_size);
	
	if (real_size < 0)
		exit(EXIT_FAILURE);

	unlink(FIFO_CONTROL_NAME);
	close(fd_src);
	close(fd_fifo);
	
	return 0;
}

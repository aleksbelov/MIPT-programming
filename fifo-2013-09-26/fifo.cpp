#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>


const char* SYNC_FIFO_NAME = "sync.fifo";
const int NAME_LENGTH = 32;
const int BUFFER_SIZE = 1024;

int main(int argc, char* argv[]) {
	if (argc == 2) {
// writer
		int inputfd = open(argv [1], O_RDONLY);
		if (inputfd < 0) {
			perror("Unable to open source file!!!");
			exit(EXIT_FAILURE);
    }
    int pid = getpid();
    char name_buffer[NAME_LENGTH];
    sprintf(name_buffer, "%d", pid);
		for (int i = strlen(name_buffer); i < NAME_LENGTH - 1; i++)
			name_buffer[i] = '!';
		name_buffer[NAME_LENGTH - 1] = '\0';
		
		//data fifo
		if ((mkfifo (name_buffer, 0644)) && (errno != EEXIST)){                         
			perror ("buffer fifo");
			exit (EXIT_FAILURE);
		}
		//temporary descriptor
		int tmp = open (name_buffer, O_RDONLY | O_NONBLOCK);
		if (tmp < 0){
			perror("Can't open FIFO");
			exit(EXIT_FAILURE); 
		} 
		int data_fifo = open (name_buffer, O_WRONLY | O_NONBLOCK);
		if (data_fifo < 0) {
			perror("Can't open FIFO");
			exit(EXIT_FAILURE); 
		}
		close(tmp);     
		
		//sycn_fifo
		if ((mkfifo (SYNC_FIFO_NAME, 0644)) && (errno != EEXIST)){                         
			perror ("Can't create FIFO");
			exit (EXIT_FAILURE);
		}
		int sync_fifo = open (SYNC_FIFO_NAME, O_WRONLY);
		if (sync_fifo < 0){
			perror("Can't open FIFO");
			exit(EXIT_FAILURE); 
		}
		
		
		if (write(sync_fifo, name_buffer, NAME_LENGTH) < NAME_LENGTH) {
			perror("Unable to write pid to sync.fifo");
			exit(EXIT_FAILURE);
		}
		close(sync_fifo);
		
		sleep(1);
		
		//change flag
		int flag = fcntl (data_fifo, F_GETFL);
		flag = flag & ~O_NONBLOCK;
		fcntl(data_fifo, F_SETFL, flag);
        
		char buffer [BUFFER_SIZE] = {}; 
		int real_size = 1;   
		while(real_size) {
			real_size = read(inputfd, buffer, BUFFER_SIZE);
			real_size = write(data_fifo, buffer, real_size);
		}
		close(data_fifo);
		close(inputfd);
		unlink(name_buffer);
	}
	else {
//reader
		//sync fifo
		if ((mkfifo (SYNC_FIFO_NAME, 0644)) && (errno != EEXIST)){                         
			perror ("Can't create FIFO");
			exit (EXIT_FAILURE);
		}
		int sync_fifo = open (SYNC_FIFO_NAME, O_RDONLY);
		if (sync_fifo < 0){
			perror("Can't open FIFO");
			exit(EXIT_FAILURE); 
		}
		
		char name_buffer [NAME_LENGTH] = {};
		int real_size = 0;
		while (real_size < NAME_LENGTH) {
			real_size = read(sync_fifo, name_buffer, NAME_LENGTH);
			if (real_size < 0) {
				perror ("Reader: unable to read from sync.fifo");
				exit(EXIT_FAILURE);
			}
		}
		close(sync_fifo);
		
		//data fifo
		int data_fifo = open (name_buffer, O_RDONLY | O_NONBLOCK);
		if (data_fifo < 0) {
			perror("Can't open FIFO");
			exit(EXIT_FAILURE); 
		}
		
		//change flag
		int flag = fcntl (data_fifo, F_GETFL);
		flag = flag & ~O_NONBLOCK;
		fcntl(data_fifo, F_SETFL, flag);

		
		char buffer [BUFFER_SIZE] = {};
		real_size = 1;
		while (real_size) {
			real_size = read(data_fifo, buffer, BUFFER_SIZE);
			real_size = write(1, buffer, real_size);
		}
		
		close(data_fifo);    
	}
	
	exit(EXIT_SUCCESS);
}

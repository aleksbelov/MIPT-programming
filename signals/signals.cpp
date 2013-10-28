#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIGONE SIGUSR1
#define SIGZERO SIGUSR2

int isgetsig = 0, sig = 0, term = 0;

const int BUFSIZE = 16;

int choose_sig(int x){
	if (x == 0) return SIGZERO;
	return SIGONE;
	
}

void getone(int x){
	isgetsig = 1;
	sig = 1;
}


void getzero(int x){
	isgetsig = 1;
	sig = 0;
}

void donone(int x){
	printf("I am donone\n");
}

void setterm(int x){
	term = 1;
}

int main(int argc, char* argv[]){
	
	if (argc > 2){
		printf("Too many arguments");
		exit(EXIT_FAILURE);
	}

	sigset_t data;
	sigemptyset(&data);
	sigaddset(&data, SIGONE);
	sigaddset(&data, SIGZERO);

	struct sigaction sigone;
	memset(&sigone, 0, sizeof(sigone));
	sigone.sa_handler = getone;
	sigone.sa_mask = data;
	sigaction(SIGONE, &sigone, NULL);
	
	struct sigaction sigzero;
	memset(&sigzero, 0, sizeof(sigzero));
	sigzero.sa_handler = getzero;
	sigzero.sa_mask = data;
	sigaction(SIGZERO, &sigzero, NULL);
	
	struct sigaction sigout;
	memset(&sigout, 0, sizeof(sigout));
	sigout.sa_handler = setterm;
	sigaction(SIGCHLD, &sigout, 0);
	
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	
	unsigned char a = 0;
	unsigned char mask = 1;
	
	sigprocmask(SIG_BLOCK, &set, NULL);
	
	int parentpid = getpid();
	int pid = fork();
	if (pid == 0){//child-writer

		struct sigaction getok;
		memset(&getok, 0, sizeof(getok));
		sigzero.sa_handler = donone;
		sigaction(SIGUSR1, &getok, NULL);
		
		sigset_t sync;
		sigemptyset(&sync);
		sigaddset(&sync, SIGUSR1);
		
		int status;
		
		int fd_source = open(argv[1], O_RDONLY);
		int size = 1;
		while (size = read(fd_source, &a, 1) > 0)
		{
			for (int i = 0; i < 8; i++){
				
				sigprocmask(SIG_BLOCK, &sync, NULL);
				
				if (!kill(parentpid, 0)) kill(parentpid, choose_sig(a & mask) );
				else {
					close(fd_source); 
					exit(EXIT_FAILURE);
				}
				a = a >> 1;
				
				sigwait(&sync, &status);
			}
		}
		close(fd_source);
		exit(0);
	}
	else if (pid == -1){
			perror("fork");
			exit(EXIT_FAILURE);
	}
	//parent-reader
	
	sigprocmask(SIG_UNBLOCK, &set, NULL);
	
	int bitcount = 0;
	int bit = 0;
	while (!(term)){
		if (isgetsig){
			sigprocmask(SIG_BLOCK, &data, NULL);
			
			if (sig == 1) a = a | mask;
			isgetsig = 0;
			mask = mask << 1;
			bitcount++;
			if (bitcount == 8){
				mask = 1;
				printf("%c", a);
				fflush(stdout);
				bitcount = 0;
				a = 0;
				
			}
			kill(pid, SIGUSR1);
		
			sigprocmask(SIG_UNBLOCK, &data, NULL);
		}

	}
	
	printf("\n");
	return 0;	
}

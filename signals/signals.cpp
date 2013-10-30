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
int parentpid;

const int ALARMTIME = 1;
const int BUFSIZE = 1;

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

void offalarm(int x){
	alarm(ALARMTIME);
}

void setterm(int x){
	term = 1;
}

void killbyalarm(int x){
	if (!kill(parentpid, 0))
		kill(parentpid, SIGKILL);
	printf("\n\n***Process-writer (parent) is not responding and was killed. Operation failed!***\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]){
	
	if (argc > 2){
		printf("Too many arguments");
		exit(EXIT_FAILURE);
	}
	parentpid = getpid();
	
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
	
	int pid = fork();
	if (pid == 0){//child-writer

		struct sigaction getok;
		memset(&getok, 0, sizeof(getok));
		getok.sa_handler = offalarm;
		sigaction(SIGUSR1, &getok, NULL);
		
		struct sigaction alarmkill;
		memset(&alarmkill, 0, sizeof(alarmkill));
		alarmkill.sa_handler = killbyalarm;
		sigaction(SIGALRM, &alarmkill, NULL);
		
		sigset_t sync;
		sigemptyset(&sync);
		sigaddset(&sync, SIGUSR1);
		
		int status;
		
		int fd_source = open(argv[1], O_RDONLY);
		//int size = 1;
		//char buf[BUFSIZE] = {};
		//size = read(fd_source, &buf, BUFSIZE);
		//while(size > 0)
		while (read(fd_source, &a, 1) > 0)
		{
			//for (int j = 0; j < size; j++){
				//a = buf[j];
				for (int i = 0; i < 8; i++){
				
					sigprocmask(SIG_BLOCK, &sync, NULL);
				
					if (!kill(parentpid, choose_sig(a & mask))) {
						alarm(ALARMTIME);
					}
					else {
						close(fd_source); 
						exit(EXIT_FAILURE);
					}
					a = a >> 1;
				
					sigwait(&sync, &status);
				}
			//}
			//size = read(fd_source, &buf, BUFSIZE);
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

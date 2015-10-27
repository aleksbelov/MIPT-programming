#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[])
{
	int pid = fork();
	if (pid == 0){
		execvp("./a.out", argv);
	}
	sleep(1);
	kill(pid, SIGKILL);
}

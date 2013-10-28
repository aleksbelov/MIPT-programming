#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
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

const int BUF_SIZE = 16;

struct msg {
	long mtype;
	char mtext[BUF_SIZE];
};

int main (int argc, char* argv[])
{
	int N = getnum(argc, argv);	
	if (N < 0)
		exit(EXIT_FAILURE);	
	
	int msgid = msgget (IPC_PRIVATE, 0666 | IPC_CREAT | IPC_EXCL);
	if (msgid == -1)
		perror("msgget error");
	
	
	for (int i = 1; i <= N; i++){
		int pid = fork();
		if (pid == 0){
			msg msgbuf;
			msgbuf.mtype = i;
			if (i != 1)
				if (msgrcv(msgid, &msgbuf, sizeof(struct msg) - sizeof(long), i, 0) == -1)
					perror("child`s msgrcv error");
			printf("%i ", i);
			fflush(stdout);
			msgbuf.mtype = i + 1;
			if (msgsnd(msgid, &msgbuf, sizeof(struct msg) - sizeof(long), 0) == -1)
				perror("child`s msgsnd error");
			fflush(stdout);
			exit(0);
		}
	}	
	
	msg msgbuf;
	msgbuf.mtype = 1;
		
	if (msgrcv(msgid, &msgbuf, sizeof(struct msg) - sizeof(long), N+1, 0) == -1)
		perror("parent`s msgrcv error");
	
	msqid_ds buf_0;
	if (msgctl(msgid, IPC_RMID, &buf_0) == -1)
		perror("msgctl error");
	printf("\n");	

	return 0;
}

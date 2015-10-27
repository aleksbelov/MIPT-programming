#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
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



struct msg {
	long mtype;
	int a;
};

int main (int argc, char* argv[])
{
	int N = getnum(argc, argv);	
	if (N < 0)
		exit(EXIT_FAILURE);


	msg msgpost[N+1];
	msgpost[0].mtype = N+1;
	int msgid = msgget (IPC_PRIVATE, 0644 | IPC_CREAT);
	
	
	for (int i = N; i > 0; i--){
		int pid = fork();
		if (pid == 0){
			int buf = 0;
			long mtype = i;
			msgrcv(msgid, &buf, sizeof(msg) - sizeof(long), mtype, 0);
			printf("%i ", i);
			fflush(stdout);
			msgsnd(msgid, &msgpost[0], sizeof(msg) - sizeof(long), 0);
			exit(0);
		}
	}

	
	for (int i = N; i > 0; i--){
		msgpost[i].mtype = i;
		msgpost[i].a = i;
		msgsnd(msgid, &msgpost[i], sizeof(msg) - sizeof(long), 0);
		
		int buf = 0;
                msgrcv(msgid, &buf, sizeof(msg) - sizeof(long), N+1, 0);
                        
	}
	return 0;
}

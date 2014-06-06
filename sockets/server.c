#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <fcntl.h>
#include <pthread.h>

#include "sockets.h"

double f(double x){
	return x*sin(x);
}

typedef struct{
	pthread_t thr;
	int n;
}calc_thread;

int Nthr = 1, Nparts = 40000000;
double sums[100];
double A, B;

void* Simpson_integral2(void* args){
	int  thr_index= *((int*)args);

	double step = ((double)B - (double)A) / Nparts;
	double a = (double)A + thr_index*((double)B - (double)A) / Nthr;
	double b = a + step;
	double BB = (double)A + (thr_index+1)*((double)B - (double)A) / Nthr;
	double sum = 0;
	printf("Thread %i: Integrate from %lg to %lg by dividing [%lg, %lg] on %i parts\n", thr_index, a, BB, a, BB, Nparts/Nthr);
	while( a < BB ){
		sum += (b-a)/6*(f(a)+4*(f((a+b)/2))+f(b));
		a += step;
		b += step;
	}
	sums[thr_index] = sum;
	return NULL;
}



int main(){

	puts("\n###\tServer started");

	//Create socket
    int brcstsock = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
    if (brcstsock == -1)
    {
    	puts("Could not create socket..");
    	puts("Terminating..");
		return 1;
    }

    //Prepare the sockaddr_in structure
    struct sockaddr_in server , client;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(1030);

    //Bind
    if( bind(brcstsock,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Broadcast socket bind failed. Error");
        return 1;
    }

    char recvmsg[1024] = {0};
    unsigned int fromLength = sizeof(client);

    while (1){
		printf("\n");
		puts("Waiting for broadcast messages...");
		if (recvfrom(brcstsock, recvmsg, sizeof(recvmsg), 0, (struct sockaddr *)&client, &fromLength) < 0)
			perror("recvfrom");

		//printf("Recieved message from client:\n\taddr: %s\n\tport: %u\n",inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		//printf("Message: %s\n", message_from_client);
		puts("Broadcast received");

		sendto(brcstsock, recvmsg, strlen(recvmsg), 0, (struct sockaddr*) &client, sizeof(client));

		//Tcp connection
		int tcpsock = socket(AF_INET , SOCK_STREAM , 0);
		if (tcpsock == -1)
		{
			puts("Could not create tcp socket..");
			puts("Terminating..");
			return 1;
		}
		int x = 1;
		setsockopt(tcpsock, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x));


		if( bind(tcpsock,(struct sockaddr *)&server , sizeof(server)) < 0)
		{
			perror("Tcp socket bind failed. Error");
			return 1;
		}

		listen(tcpsock , 1);

		//Accept and incoming connection

		printf("");
		puts("Waiting for incoming connections...");

		fcntl(tcpsock, F_SETFL, O_NONBLOCK);
		fd_set readset;
        FD_ZERO(&readset);
        FD_SET(tcpsock, &readset);

        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        if (select(tcpsock + 1, &readset, NULL, &readset, &timeout) == 0)
        	printf("No tasks during 5s ..\n");
        else {
            int c = sizeof(struct sockaddr_in);
            int client_sock = accept(tcpsock, (struct sockaddr *)&client, (socklen_t*)&c);
            if (client_sock < 0)
            {
                perror("accept failed");
            }
            else {
				puts("Connection accepted");

				data_for_server task;
				recv(client_sock, &task, sizeof(task), 0);
				printf("Task received:\n"
					   "  Integrate from %lg to %lg\n", task.a, task.b);

				A = task.a;
				B = task.b;
				Nthr = task.nthrs;
				Nparts = task.nparts;

				calc_thread* calculators = malloc(sizeof(calc_thread)*task.nthrs);
				for (int i = 0; i < task.nthrs; i++){
					calculators[i].n = i;
					if (pthread_create(&calculators[i].thr, NULL, Simpson_integral2, &calculators[i].n) < 0){
						printf("Creating thread %i\n\t", i);
						perror("Error");
					}
				}

				for (int i = 0; i < task.nthrs; i++)
					pthread_join(calculators[i].thr, NULL);

				double result = 0.0;
				for (int i = 0; i < task.nthrs; i++)
					result += sums[i];
				printf("Completed. Result = %lg\n", result);
				if (send(client_sock, &result, sizeof(result), 0) < 0)
					perror("Sending results. Error");

				close(client_sock);
				free(calculators);
            }
        }
        close(tcpsock);
    }
    return 0;
}

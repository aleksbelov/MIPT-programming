с
#include <sys/socket.h>
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include "sockets.h"

int SERVER_CRASHED = -1;
int COUNT_OF_THREADS = 2;
int COUNT_OF_SERVERS = 0;
int COUNT_OF_PARTS = 100000000;
int REAL_COUNT_OF_SERVERS = 0;
double A = 0.0, B = 1.0;

typedef struct {
	int nthrs, *server_error;
	struct sockaddr_in *server_addr;
	double *result, a, b;
} thread_args_t;

void* connect_with_one_server(void* args){
	thread_args_t my_args = *(thread_args_t*)args;

	//Tcp connection
	int tcpsock = socket(PF_INET , SOCK_STREAM , 0);
	if (tcpsock == -1)
	{
		puts("Could not create tcp socket..");
		puts("Terminating..");
		return NULL;
	}

	int optval = 1;
	setsockopt(tcpsock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
	optval = 1;
	setsockopt(tcpsock, IPPROTO_TCP, TCP_KEEPCNT, &optval, sizeof(optval));
	optval = 2;
	setsockopt(tcpsock, IPPROTO_TCP, TCP_KEEPIDLE, &optval, sizeof(optval));
	optval = 1;
	setsockopt(tcpsock, IPPROTO_TCP, TCP_KEEPINTVL, &optval, sizeof(optval));

	puts("Connecting with server..");
	if (connect(tcpsock, (struct sockaddr *)my_args.server_addr, sizeof(*(my_args.server_addr))) < 0){
		perror("Connection failed! Error");
		exit(1);
		}
	else
		puts("Connection was established");

	//Sending task
	data_for_server sendargs;
	sendargs.a = my_args.a;
	sendargs.b = my_args.b;
	sendargs.nthrs = my_args.nthrs;
	sendargs.nparts = COUNT_OF_PARTS/REAL_COUNT_OF_SERVERS;

	if (send(tcpsock, &sendargs, sizeof(sendargs), 0) < 0){
		perror("Sending data to server failed");
		*my_args.result = 0.0;
		*my_args.server_error = 1;
		return NULL;
	}

	printf("Send task: success. Server is calculating..\n");
	double result;
	int recv_res = recv(tcpsock, &result, sizeof(result), 0);

	if (recv_res <= 0){
		*my_args.result = 0.0;
		*my_args.server_error = 1;
		return NULL;
	}
	*my_args.server_error = 0;
	*my_args.result = result;
	close(tcpsock);
	return NULL;
}

int broadcast_search_for_servers(struct sockaddr_in *server_addr){
	//Create broadcast socket
	int brcstsock = socket(PF_INET , SOCK_DGRAM, 0);
	if (brcstsock == -1)
	{
		puts("Could not create broadcast socket..");
		close(brcstsock);
		return 0;
	}

	struct sockaddr_in brcst_addr;
	brcst_addr.sin_family = AF_INET;
	brcst_addr.sin_port = htons(1030);
	brcst_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	// Set up a broadcast
	int x = 1;
	setsockopt(brcstsock, SOL_SOCKET, SO_BROADCAST, &x, sizeof(x));


	char msgb[] = "Hello, server!";

	printf("Sending message to servers:\n\taddr: %s\n\tport: %u\n",inet_ntoa(brcst_addr.sin_addr), ntohs(brcst_addr.sin_port));

	if (sendto(brcstsock, msgb, strlen(msgb), 0, (const struct sockaddr*) &brcst_addr, sizeof(brcst_addr)) < 0){
		perror("Sending broadcast. Error");
		close(brcstsock);
		return 0;
	}
	fcntl(brcstsock, F_SETFL, O_NONBLOCK);
	// Select settings
	fd_set readset;
	FD_ZERO(&readset);
	FD_SET(brcstsock, &readset);

	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;


	int servcount = 0;
	char msgbr[1024] = {0};
	socklen_t servaddrlen = sizeof(struct sockaddr_in);

	//Search for servers
	while(servcount < COUNT_OF_SERVERS && select(brcstsock + 1, &readset, NULL, &readset, &timeout) > 0) {
		recvfrom(brcstsock, msgbr, sizeof(msgbr), 0,
		  (struct sockaddr*) &server_addr[servcount],
		  &servaddrlen);

		if(strcmp(msgbr, msgb) == 0) {
			servcount++;
		  FD_ZERO(&readset);
		  FD_SET(brcstsock, &readset);
		}
	  }

	close(brcstsock);
	return servcount;
}


int main(int argc, char* argv[]){

	if (argc < 4){
		printf("Usage: %s [number of servers] [number of threads on a server] [number of parts to divide interval on]\n", argv[0]);
		exit(1);
	}
	COUNT_OF_SERVERS = atoi(argv[1]);
	COUNT_OF_THREADS = atoi(argv[2]);
	COUNT_OF_PARTS = atoi(argv[3]);

	double *results;
	int *server_error;

	int done;
	do {
		done = 1;
		struct sockaddr_in *server_addr = calloc(COUNT_OF_SERVERS, sizeof(struct sockaddr_in));

		int servcount = broadcast_search_for_servers(server_addr);

		printf("  %i server(s) found during <= 2s\n", servcount);
		if (!REAL_COUNT_OF_SERVERS){
			REAL_COUNT_OF_SERVERS = servcount;
			results = calloc(servcount, sizeof(double));
			server_error = calloc(servcount, sizeof(int));
			for (int i = 0; i < servcount; i++)
				server_error[i] = 1;
		}
		if (servcount == 0){
			done = 0;
			break;
		}

		int servers_i = 0;
		pthread_t thr_id[4];
		thread_args_t thread_to_server_args[4];
		for(int i = 0; i < REAL_COUNT_OF_SERVERS; i++) {
			if (server_error[i]){
				thread_to_server_args[i].a = A + (B-A)/(REAL_COUNT_OF_SERVERS)*i;
				thread_to_server_args[i].b = A + (B-A)/(REAL_COUNT_OF_SERVERS)*(i+1);
				thread_to_server_args[i].nthrs = COUNT_OF_THREADS;
				thread_to_server_args[i].result = &results[i];
				thread_to_server_args[i].server_addr = &server_addr[servers_i];
				servers_i++;
				thread_to_server_args[i].server_error = &server_error[i];

				if(pthread_create(&thr_id[i], NULL, connect_with_one_server, &thread_to_server_args[i]) != 0){
					printf("Create send %i thread failed.. ", i);
					perror("");
				}
			}
		}

		for (int i = 0; i < REAL_COUNT_OF_SERVERS; i++)
			if (server_error[i])
				pthread_join(thr_id[i], NULL);

		COUNT_OF_SERVERS = 0;

		for (int i = 0; i < servcount; i++){
			if (server_error[i]){
				printf("Server %i crashed..\n", i);
				done = 0;
				COUNT_OF_SERVERS++;
			}
			else
				printf("Server %i: success. Result: %lg\n", i, results[i]);
		}

		free(server_addr);
	}while(!done);

	for (int i = 1; i < REAL_COUNT_OF_SERVERS; i++)
		results[0] += results[i];

	if (REAL_COUNT_OF_SERVERS > 0 && done == 1)
		printf("Results received successfully:\n\tIntegral = %lg\n", results[0]);
	else
		printf("Errors occurred. No results..\n");
	return 0;
}

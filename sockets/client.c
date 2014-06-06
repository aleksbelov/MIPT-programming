#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <fcntl.h>
#include <pthread.h>

#include "sockets.h"

int SERVER_CRASHED = -1;
int COUNT_OF_THREADS = 2;
int COUNT_OF_SERVERS = 0;
int COUNT_OF_PARTS = 100000000;
int REAL_COUNT_OF_SERVERS = 0;

typedef struct {
	int nthrs, *server_error;
	struct sockaddr_in *server_addr;
	double *result, a, b;
} thread_args_t;

void* connect_with_one_server(void* args){
	thread_args_t my_args = *(thread_args_t*)args;

	//Tcp connection
	int tcpsock = socket(AF_INET , SOCK_STREAM , 0);
	if (tcpsock == -1)
	{
		puts("Could not create tcp socket..");
		puts("Terminating..");
		return NULL;
	}

	struct timeval timeout;
	timeout.tv_sec = 18;
	timeout.tv_usec = 0;

	//apply send timeout socket options
	//setsockopt(tcpsock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
	int optval = 1;
	int rez_setsock = setsockopt(tcpsock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
	setsockopt(tcpsock, SOL_SOCKET, TCP_KEEPCNT, &optval, sizeof(optval));
	setsockopt(tcpsock, SOL_SOCKET, TCP_KEEPIDLE, &optval, sizeof(optval));
	setsockopt(tcpsock, SOL_SOCKET, TCP_KEEPINTVL, &optval, sizeof(optval));

	//printf("setsockopt: %i", rez_setsock);


	puts("Connecting with server..");
	if (connect(tcpsock, (struct sockaddr *)my_args.server_addr, sizeof(*(my_args.server_addr))) < 0)
		perror("Connection failed! Error");
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

	//printf("recv: %i\n", recv_res);
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
	int brcstsock = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
	if (brcstsock == -1)
	{
		puts("Could not create broadcast socket..");
		puts("Terminating..");
		return 0;
	}

	// Set up a broadcast
	int x = 1;
	setsockopt(brcstsock, SOL_SOCKET, SO_BROADCAST, &x, sizeof(x));

	struct sockaddr_in brcst_addr;
	brcst_addr.sin_family = AF_INET;
	brcst_addr.sin_port = htons(1030);
	brcst_addr.sin_addr.s_addr = INADDR_BROADCAST;



	char msgb[] = "Hello, server!";

	if (sendto(brcstsock, msgb, strlen(msgb), 0, (struct sockaddr*) &brcst_addr, sizeof(brcst_addr)) < 0){
		perror("Sending broadcast. Error");
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
		int rdbyte = recvfrom(brcstsock, msgbr, sizeof(msgbr), 0,
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
		//printf("First/now count of servers = %i/%i\n", REAL_COUNT_OF_SERVERS, servcount);

		int servers_i = 0;
		pthread_t thr_id[4];
		thread_args_t thread_to_server_args[4];
		for(int i = 0; i < REAL_COUNT_OF_SERVERS; i++) {
			if (server_error[i]){
				//printf("Creating connection to calculate [%i]\n", i);
				thread_to_server_args[i].a = 1.0/(REAL_COUNT_OF_SERVERS)*i;
				thread_to_server_args[i].b = 1.0/(REAL_COUNT_OF_SERVERS)*(i+1);
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
		//for (int i = 0; i < servcount; i++)
			//printf("server[%i]: error = %i\n", i, server_error[i]);
		for (int i = 0; i < servcount; i++){
			//printf("server_error[%i] = %i\n", i, server_error[i]);
			if (server_error[i]){
				printf("Server %i crashed..\n", i);
				done = 0;
				COUNT_OF_SERVERS++;
			}
			else
				printf("Server %i: success. Result: %lg\n", i, results[i]);
		}
		//if (servcount == 0)
		//	done = 1;
		//printf("done: %i\n", done);
		free(server_addr);
	}while(!done);

	for (int i = 1; i < REAL_COUNT_OF_SERVERS; i++)
		results[0] += results[i];

	if (REAL_COUNT_OF_SERVERS > 0)
		printf("Results received successfully:\n\tIntegral = %lg\n", results[0]);
	else
		printf("Errors occurred. No results..\n");
	return 0;
}

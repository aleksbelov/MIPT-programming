//
//  server.c
//  net_parallel_calculating
//
//  Created by Vadim on 21.05.14.
//  Copyright (c) 2014 kusaches. All rights reserved.
//

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>

#define PORT 51012
#define f(x) 1

struct Limits{
    double a;
    double b;
    double result;
    long long depth;
};

void * calculate(void*);

int main()
{
    struct Limits *limits;
    struct Limits recv_limits;
    double dx;
    double result;
    int broadcast_socket, tcp_socket, socket_connect, i, x;
    struct sockaddr_in addr, client_addr;
    char buffer[1000];
    pthread_t *threads;
    socklen_t len = sizeof(client_addr);
    int cpus = 2;
    cpus = sysconf(_SC_NPROCESSORS_ONLN);
while( 1 )
{
    /************************************************/
    //tcp socket creation
    
    tcp_socket = socket(PF_INET, SOCK_STREAM, 0);
    if ( tcp_socket == -1 )
    {
        return 2;
    }
    
    /************************************************/
    //from broadcast socket block
    broadcast_socket = socket(PF_INET, SOCK_DGRAM, 0); //udp socket
    if ( broadcast_socket == -1 )
    {
        return 2;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if ( bind(broadcast_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1 )
    {
        perror("Binding tcp socket");
        close(broadcast_socket);
        return 3;
    }
    
    if( recvfrom(broadcast_socket, buffer, 1000, 0, (struct sockaddr*)&client_addr, &len) == -1 )
    {
        perror("recieve from broadcast");
        close(broadcast_socket);
        return 4;
    }
    
    if( sendto(broadcast_socket, &cpus, sizeof(cpus), 0, (struct sockaddr*)&client_addr, len) == -1 )
    {
        perror("sendto");
        close(broadcast_socket);
        return 4;
    }

    
    /***********************************************/
    //tcp connection block
    printf("tcp connection block reached\n");
    
    close(broadcast_socket);
    i = 1;
    if( setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i) ) == -1 )
    {
        perror("setsockopt");
        return -1;
    }
    
    if ( bind(tcp_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1 )
	  {
        perror("Binding tcp socket");
        close(tcp_socket);
        return 3;
	  }
    if ( listen(tcp_socket, 1) == -1)
    {
        perror("Listening");
        close(tcp_socket);
        return 5;
    }


    socket_connect = accept( tcp_socket, (struct sockaddr *)&client_addr, &len);
    if ( socket_connect == -1 )
    {
        perror("Accept tcp socket");
        close(socket_connect);
        close(tcp_socket);
        return 6;
    }

    
    /*********************************************/
    //calculating block
    printf("calculating block reached\n");
    limits = (struct Limits*)malloc(sizeof(struct Limits) * cpus);
    threads = (pthread_t*)malloc( sizeof( pthread_t) * cpus);
    if( !threads || !limits )
        return 2;
recalculation:
	printf("calculation\n");
 	recv(socket_connect, &recv_limits, sizeof(recv_limits), 0);
	   
	dx = (recv_limits.b - recv_limits.a) / cpus;
    for( i = 0; i < cpus; i++ )
    {
	limits[i].a = recv_limits.a + i * dx;
	limits[i].b = limits[i].a + dx;
	limits[i].depth = recv_limits.depth / cpus;
        pthread_create(&threads[i], NULL, calculate, &limits[i]);
        
    }
    for( i = 0; i < cpus; i++ )
        pthread_join( threads[i], NULL);
    
    result = 0.0;
    for( i = 0; i < cpus; i++ )
        result += limits[i].result;

    if((x =  send(socket_connect, &result, sizeof(result), 0)) == -1 )
        perror("send");
    /***********************************************************/
    //recalculation block

	recv(socket_connect, &i, sizeof(i), 0);
	if( i )
		goto recalculation;
	close(tcp_socket);

    
    printf("ready\n");
    
}
    return 0;
}


void * calculate(void *arg)
{
	struct Limits *p = (struct Limits*)arg;
	long long  i, N = p->depth;
	double dx, x, sum;
	sum = 0.0;
    
	dx = (p->b - p->a) / N;
	x = p->a;
	sum += (f(x) + f(x+dx)) * dx / 2;     //trapezium method
	for( i = 1; i < N; i++ )
	{
		x += dx;
		sum += (f(x) + f(x+dx)) * dx / 2;
	}
	p->result = sum;
	return NULL;
}

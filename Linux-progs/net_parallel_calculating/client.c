//
//  client.c
//  net_parallel_calculating
//
//  Created by Vadim on 21.05.14.
//  Copyright (c) 2014 kusaches. All rights reserved.
//

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <strings.h>

#define PORT 51012
#define SLEEP_TIME 1
#define N 10000000000

struct Limits{
    double a;
    double b;
    double result;
    long long depth;
};



struct List{
    struct sockaddr_in server;  //list of servers and their number of cpus
    int cpus;
    socklen_t len;
};

void print_list(struct List*, int);
struct List * creat_record(struct List*, struct sockaddr_in*, int, socklen_t);



int main(int argc, char** argv)
{
    if ( argc < 3 )
        return 1;       //min 2 args:[a, b]
    /**********************************************************/
    // declaration of variables block
    int broadcast_socket, number_of_cpus = 0;
    int *tcp_sockets;      // one tcp socket for each server
    int number_of_servers, i, bc = 1;
    double a, b, dx, result = 0.0;          // the ends of the segment
    struct sockaddr_in addr, server_addr, *addrs;
    socklen_t len = sizeof(addr);
    char msg[] = "Calculate\n";
    int cpus;
    struct List list[10];
    int bad_tcp_sockets[10];
    struct Limits *limits;
    int *cpus_all, k = 0, j = 0, x;
    long long  depth;
    for(i = 0; i < 10; i++ )
        bad_tcp_sockets[i] = -1;
    printf("broadcast block reached\n");
    //broadcast
    /*********************************************************/
    broadcast_socket = socket(PF_INET, SOCK_DGRAM, 0); //udp socket
    if ( broadcast_socket == -1 )
    {
        perror("socket");
        return 2;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl( INADDR_BROADCAST);
    
    if ( setsockopt( broadcast_socket, SOL_SOCKET, SO_BROADCAST, ( const void *) &bc, sizeof( bc)) == -1)
	{
    		 perror("setsockopt");
		close( broadcast_socket);
		return 3;
	}

    if( (sendto(broadcast_socket, msg, sizeof(msg), 0, (struct sockaddr*)&addr, len)) == -1 )
    {
        perror("sendto");
    }
    
    /*********************************************************/
    //waiting for the answers from servers
    printf("waiting block reached\n");
    
    if( fcntl(broadcast_socket, F_SETFL, O_NONBLOCK) == -1 )
    {
        perror("fcntl");
        return 4;
    }
    sleep(SLEEP_TIME);
    /**************************************************/
    //recieve ready servers and take the number of cpus
    printf("servers block reached\n");
    i = 0;
    while( recvfrom(broadcast_socket, &cpus, sizeof(cpus), 0, (struct sockaddr*)&server_addr, &len) != -1 )
    {
        number_of_cpus += cpus;
        creat_record(&list[i], &server_addr, cpus, len);
        i++;
    }
   
    putchar('\n');
    printf("List of servers:\n");
    print_list(list, i);
    putchar('\n');
    close(broadcast_socket);
    if( !i )
    {
        printf("No servers\n");
        return 1;
    }
    /**********************************************************/
    //tcp connections block
    printf("tcp connections block reached\n");
    
    number_of_servers = i;
    tcp_sockets = (int*)malloc(sizeof(int) * number_of_servers);
    addrs = (struct sockaddr_in*)malloc(number_of_servers * sizeof(struct sockaddr_in));
    cpus_all = (int*)malloc(sizeof(int) * number_of_servers);
 
    if( (tcp_sockets == NULL) || (addrs == NULL) || (cpus_all == NULL) )
    {
        perror("malloc");
        return 6;
    }
    for(i = 0; i < number_of_servers; i++ )
    {
        tcp_sockets[i] = socket(PF_INET, SOCK_STREAM, 0);
        if( tcp_sockets[i] == -1 )
        {
            perror("socket");
            return 2;
        }
        
	
        addrs[i].sin_family = AF_INET;
        addrs[i].sin_addr.s_addr = INADDR_ANY;
        addrs[i].sin_port = 0;
        
        if ( bind(tcp_sockets[i], (struct sockaddr*)&(addrs[i]), sizeof(addrs[i])) == -1 )
        {
            close(tcp_sockets[i]);
            perror("bind");
            return 3;
        }
	bc = 1;
	setsockopt(tcp_sockets[i], SOL_SOCKET, SO_KEEPALIVE, &bc, sizeof(bc));
	bc = 1;
	setsockopt(tcp_sockets[i], SOL_TCP, TCP_KEEPCNT, &bc, sizeof(bc));
	bc = 2;
	setsockopt(tcp_sockets[i], SOL_TCP, TCP_KEEPIDLE, &bc, sizeof(bc));
	bc = 1;
	setsockopt(tcp_sockets[i], SOL_TCP, TCP_KEEPINTVL, &bc, sizeof(bc));
	 
        len = sizeof(struct sockaddr_in);

        if ( connect(tcp_sockets[i], (struct sockaddr*)&(list[i].server), len) == -1 )
        {
            perror("Connection failed");
            close( tcp_sockets[i]);
            return 7;
        }
        cpus_all[i] = list[i].cpus;

    
    }
    
    /****************************************************************/
    //giving the task
    printf("task block reached\n");
    
    a = atof(argv[1]);
    b = atof(argv[2]);

    limits = (struct Limits*)malloc(number_of_cpus * sizeof(struct Limits));
    if( limits == NULL )
    {
        perror("malloc");
        return 7;
    }
    dx = (b - a) / number_of_cpus;
    depth = N / number_of_cpus;
    
	k = 0;
	for( i = 0; i < number_of_servers; i++ )
	{
		limits[i].a = a + dx * k;
		limits[i].b = limits[i].a + dx * cpus_all[i];
		limits[i].depth = depth * cpus_all[i];
		k += cpus_all[i];
		if( send(tcp_sockets[i], &limits[i], sizeof(limits[i]), 0) == -1 )
		{
			perror("send");
		}
	}


    printf("waiting for result\n");
    k = 0;
    for( i = 0; i < number_of_servers; i++ )
    {
        x = recv(tcp_sockets[i], &limits[i].result, sizeof(limits[i].result), 0);
        result += limits[i].result;
        if( (x == 0) || (x == -1) )
        {
            result -= limits[i].result;
            bad_tcp_sockets[k] = i;
            close(tcp_sockets[i]);
            tcp_sockets[i] = -1;
            number_of_cpus -= cpus_all[i];
            k++;
        }
    }
    /****************************************************************/
    //recalculation block
    
    if( !k )
        bc = 0;
    else
        bc = 1;
    for( i = 0; i < number_of_servers; i++ )          // say to servers about need or not to recalculate
    {
        if( tcp_sockets[i] == -1 )
            continue;
        if( send(tcp_sockets[i], &bc, sizeof(bc), 0) == -1 )
        {
            perror("send");
        }
    }
    
    if( k )    // k is the number of failed servers
    {
        dx = (limits[k-1].b - limits[k-1].a)/number_of_cpus;
        depth = N - depth * number_of_cpus;
	depth /= number_of_cpus;
        printf("Recalculation block reached\n");
        for(k = 0, j = bad_tcp_sockets[0]; j != -1; j = bad_tcp_sockets[++k] )
        {
            x = 0;
            for( i = 0; i < number_of_servers; i++ )
            {
                if( tcp_sockets[i] == -1 )
                    continue;
                limits[i].a = limits[j].a + x * dx;
                limits[i].b = limits[i].a + dx * cpus_all[i];
                limits[i].depth = depth * cpus_all[i];
                x += cpus_all[i];
            
            	if( send(tcp_sockets[i], &limits[i], sizeof(limits[i]), 0) == -1 )
            	{
                	perror("send");
            	}
	    }
        }
        
        for( i = 0; i < number_of_servers; i++ )
        {
            if( tcp_sockets[i] == -1 )
                continue;
            x = recv(tcp_sockets[i], &limits[i].result, sizeof(limits[i].result), 0);
            result += limits[i].result;
        }

        
    }
    bc = 0;
    for( i = 0; i < number_of_servers; i++ )          // say to servers about need or not to recalculate
    {
        if( tcp_sockets[i] == -1 )
            continue;
        if( send(tcp_sockets[i], &bc, sizeof(bc), 0) == -1 )
        {
            perror("send");
        }
    }

    
    printf("result: %f\n", result);
    
    free(limits);
    free(tcp_sockets);
    free(addrs);
    free(cpus_all);  
    /***************************************************************/
    return 0;
}


struct List * creat_record(struct List *list, struct sockaddr_in *server_addr, int cpus, socklen_t len)
{
    list->cpus = cpus;
    list->server.sin_port = server_addr->sin_port;
    list->server.sin_family = server_addr->sin_family;
    list->server.sin_addr.s_addr = server_addr->sin_addr.s_addr;
    list->len = len;
    return list;
}

void print_list(struct List *list, int n)
{
	int i;
	for( i = 0; i < n; i++)
        	printf( "%d %d %d\n", list[i].server.sin_port, list[i].server.sin_addr.s_addr, list[i].cpus);
}




/****************************************************************************************
 BUGS
 

 
 SEE
 
 frees
 perror argument
 number of cpus
*/

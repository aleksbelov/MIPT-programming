#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/select.h>

const char* INPUTFILE = "input.txt";
const char* OUTPUTFILE = "output.txt";

const int BUF_SIZE = 18;
const int RD = 0;
const int WR = 1;

int getnum(int argc, char* str);
void runchild(int pipefdrd, int pipefdwr);
int closefds(int fd1, int fd2, int fds[][2], int npipes);
int buf_size(int buf_num, int nchs);
int shiftbuf(char buf[], int size, int shift);

int main(int argc, char* argv[]){
	int nchs = getnum(argc, argv[1]); //number of childs
	if (nchs < 0){
		printf("# Please, input a number of childs correctly!!!\n");
		exit(EXIT_FAILURE);
	}
	
	int pipefd[2*nchs - 1][2];
	
	int inputfd = open(INPUTFILE, O_RDONLY);
	if (inputfd == -1){
		perror("open input");
		exit(EXIT_FAILURE);
	}
	int outputfd = 1;
	/*
	int outputfd = open(OUTPUTFILE, O_WRONLY | O_CREAT);//1;//stdout
	if (outputfd == -1){
		perror("open output");
		exit(EXIT_FAILURE);
	}
	*/
	
	for (int i = 0; i < 2*nchs - 1; i++){
		if (pipe(pipefd[i]) == -1){
			perror("creating pipes");
			exit(EXIT_FAILURE);
		}
	}
	//pipes dump
	//printf("pipe deskriptors: ");
	//for (int i = 0; i < 2*nchs - 1; i++)
	//	printf("%i %i, ", pipefd[i][RD], pipefd[i][WR]);
	//printf("\ninput %d\noutput %d\nend\n", inputfd, outputfd);
	
	
	for (int i = 0; i < nchs; i++){
		int pid = fork();
		if (pid == 0){//childs algorithm
			if (i == 0){
				closefds(inputfd, pipefd[2*i][WR], pipefd, 2*nchs - 1);
				runchild(inputfd, pipefd[2*i][WR]);
			}
			
			closefds(pipefd[2*i-1][RD], pipefd[2*i][WR], pipefd, 2*nchs - 1);
			runchild(pipefd[2*i-1][RD], pipefd[2*i][WR]);
		}
		else{
			if (i != 0)
				close(pipefd[2*i-1][RD]);
			close(pipefd[2*i][WR]);
			//if (i != 0)
			//	printf("parent: closed %d %d\n",pipefd[2*i-1][RD], pipefd[2*i][WR]);
			//else printf("parent: closed %d\n",pipefd[2*i][WR]);
		}
	}
	
	fd_set rfds;
	fd_set wfds;
	
	char** buf = (char**) calloc(nchs, sizeof(char*));
	for (int i = 0; i < nchs; i++)
		buf[i] = (char*) calloc(buf_size(i, nchs), sizeof(char));
	
	int chpipes[nchs][2];
	for (int i = 0; i < nchs; i++){
		chpipes[i][RD] = pipefd[2*i][RD];
		chpipes[i][WR] = pipefd[2*i+1][WR];
		if (i == nchs - 1)
			chpipes[i][WR] = outputfd;
		//printf("%d: [%d] [%d]\n", i, chpipes[i][RD], chpipes[i][WR]);
	}
	
	
	int* buf_full = (int*) calloc (nchs, sizeof(int));
	while(1){
		
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		int selectnum = 0;
		for (int i = 0; i < nchs; i++)
			if (buf_full[i] > 0){
				FD_SET(chpipes[i][WR], &wfds);
				//printf("wfds: +[%d]\n",chpipes[i][WR]);
				if (selectnum < chpipes[i][WR])
					selectnum = chpipes[i][WR];
			}
			else if (buf_full[i] == 0){
				FD_SET(chpipes[i][RD], &rfds);
				//printf("rfds: +[%d]\n",chpipes[i][RD]);
				if (selectnum < chpipes[i][RD])
					selectnum = chpipes[i][RD];
			}
		selectnum++;
		//printf("selectnum = %d\n", selectnum);
		if (selectnum == 1)
			break;
			
		int retval = select(selectnum, &rfds, &wfds, NULL, NULL);
		if (retval == -1)
			perror("parent select");
		else{
			for (int i = 0; i < nchs; i++){
				if (FD_ISSET(chpipes[i][RD], &rfds)){
					buf_full[i] = read(chpipes[i][RD], buf[i], buf_size(i, nchs));
					if (buf_full[i] < 0)
						perror("parent read");
					if (buf_full[i] == 0){
						close(chpipes[i][RD]);
						close(chpipes[i][WR]);
						buf_full[i] = -1;
					}
					//printf("parent read %d bytes from [%d]\n", buf_full[i], chpipes[i][RD]);
				}
				if (FD_ISSET(chpipes[i][WR], &wfds)){
					int real_wrote;
					real_wrote = write(chpipes[i][WR], buf[i], buf_full[i]);
					//printf("\nparent wrote %d/%d byte to [%d]\n", real_wrote, buf_full[i], chpipes[i][WR]);
					if (real_wrote != buf_full[i])
						shiftbuf(buf[i], buf_full[i], buf_full[i] - real_wrote);
					buf_full[i] -=real_wrote;
				}	
			}
		}
		//for(int i = 0; i < nchs; i++)
			//printf("buf_full[%d] = %d\n", i, buf_full[i]);
		
		//printf("\n");
	}
	for (int i = 0; i < nchs; i++)
		close(chpipes[i][WR]);
	int status;
	wait(&status);
	close(inputfd);
	return 0;
}

//======================================================================
// CHILD
void runchild(int pipefdrd, int pipefdwr){
	//printf("child: %i %i\n", pipefdrd, pipefdwr);
	char* buf = (char*) calloc(BUF_SIZE, sizeof(char));
	int real_size = 1;
	
	while (real_size > 0){		
		real_size = read(pipefdrd, buf, BUF_SIZE);
		if (real_size < 0)
			perror("child read");
		write(pipefdwr, buf, real_size);
		//printf("child: real_size = %d\n", real_size);
	}
	//printf("real_size = %d\n", real_size);
	free(buf);
	close(pipefdrd);
	close(pipefdwr);
	exit(EXIT_SUCCESS);	
}

int shiftbuf(char buf[], int size, int shift){
	for (int i = 0; i < size; i++)
		buf[i] = buf[i+shift];
}

int closefds(int fd1, int fd2, int fds[][2], int npipes){
	for (int i = 0; i < npipes; i++){
		if (fds[i][0] != fd1 && fds[i][0] != fd2)
			close(fds[i][0]);
		if (fds[i][1] != fd1 && fds[i][1] != fd2)
			close(fds[i][1]);
	}
}

int buf_size(int buf_num, int nchs){
	return 3/*1024*/*(nchs - buf_num + 1);
}

int getnum(int argc, char* str)
{
	char *endptr = NULL;
        if (argc < 2)
			return -1;
        int N = strtol(str, &endptr, 0);

        if (*endptr != '\0')
        	return -1;
	
	return N;
}

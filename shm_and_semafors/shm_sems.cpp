#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

const int SHM_SIZE = 1024;
const int NUM_SEMS = 3;
enum {SHM = 0, WR, RD};


void sems_dump (int sem_id){
	printf("***semafor dump***\n");
	for (int i = 0; i < NUM_SEMS; i++)
		printf("semset[%d] = %d\n", i, semctl(sem_id, i, GETVAL));
	printf("***end dump***\n");
}

void init_sop(sembuf &op, int sem_num, int sem_op, int sem_flg){
	op.sem_num = sem_num;
	op.sem_op = sem_op;
	op.sem_flg = sem_flg;
}




void record_size(char* shm, int* size){
	for (int i = 0; i < 4; i++)
		shm[i] = *(((char*)size) + i);
}

int scan_size(char* shm){
	int size = 0;
	int* psize = &size;
	for (int i = 0; i < 4; i++)
		*(((char*)psize) + i) = shm[i];
	return size;
}

int writer(key_t keyshm, key_t keysem, const char* inputfile){

//======================================================================
//	Open input file
	int infd = open(inputfile, O_RDONLY);
	if (infd == -1){
		perror("#writer->open(inputfile)");
		exit(EXIT_FAILURE);
	}
	
	
//======================================================================
//	Create shared memory identifer
	
	int shm_id = shmget(keyshm, SHM_SIZE+4, IPC_CREAT | 0666);
	if (shm_id  == -1){
		perror("#writer->shmget");
		exit(EXIT_FAILURE);
	}
	
//======================================================================
//	Create semafors
	int sem_id = semget(keysem, NUM_SEMS, IPC_CREAT | 0666);
	if (sem_id == -1)
	{
		perror("#writer->semget");
		exit(EXIT_FAILURE);
	}
	
//======================================================================
//	Attaching shared memory segment
	char* shm = (char*) shmat(shm_id, NULL, 0);
	if (shm == (void*) -1){
		perror("#writer->shmat");
		exit(EXIT_FAILURE);
	}

//======================================================================
//	Synchronization


	sembuf sync[3];
	init_sop(sync[0], WR, 0, IPC_NOWAIT);
	init_sop(sync[1], WR, 2, SEM_UNDO);
	if (semop(sem_id, sync, 2) == -1){
		printf("#Sorry, another reader already exists. Terminating..\n");
		exit(EXIT_SUCCESS);
	}
	
	init_sop(sync[0], RD, -1, 0);
	semop(sem_id, &sync[0], 1);
	
	//Semafore SHM has to be "zero" after terminating process
	init_sop(sync[0], SHM, 2, SEM_UNDO);
	init_sop(sync[1], SHM, -2, 0);
	semop(sem_id, sync, 2);

//======================================================================
//	Transfering data
	
	int real_size = 1;
	int dead_reader = 0;
	while (real_size > 0){
		sembuf sop[3];
		init_sop(sop[0], RD, -1, IPC_NOWAIT);
		init_sop(sop[1], RD, 1, 0);
		init_sop(sop[2], SHM, 0, 0);
		if (semop(sem_id, sop, 3) == -1){
			printf("#Sorry, reader was suddenly terminated. Terminating..\n");
			dead_reader = 1;
			break;
		}
		
		real_size = read(infd, shm, SHM_SIZE);
		record_size(shm + SHM_SIZE, &real_size);		
		
		sop[2].sem_op = 2;
		semop(sem_id, &sop[2], 1);
		
	}



//======================================================================
//	Detaching shared memory segment	
	if (shmdt(shm) == -1){
		perror("#writer->shmdt");
		exit(EXIT_FAILURE);
	}
	
	init_sop(sync[0], RD, 0, 0);
	semop(sem_id, sync, 1);//Wait for reader dead
	
	if (shmctl(shm_id, IPC_RMID, NULL) == -1){
		perror("writer->shmctl");
		exit(EXIT_FAILURE);
	}
	
	if (semctl(sem_id, 0, IPC_RMID) == -1){
		perror("#writer->semctl");
		exit(EXIT_FAILURE);
	}

	exit(0);
}



int main(int argc, char* argv[]){

	key_t keyshm = ftok(argv[0], 1);
	key_t keysem = ftok(argv[0], 2);
	if (argc == 2){ 
		writer(keyshm, keysem, argv[1]);
		exit(0);
	}
	
	if (argc > 2) {
		printf("#Error! Run: %s <src_file>\n", argv[0]); 
		exit(EXIT_FAILURE);
	}
	
	
// READER

//======================================================================
//	Create shared memory identifer	
	int shm_id = shmget(keyshm, SHM_SIZE+4, IPC_CREAT | 0666);
	/*if (shm_id  == -1){
		perror("#reader->shmget");
		exit(EXIT_FAILURE);
	}
	*/
//======================================================================
//	Create semafors	

	int sem_id = semget(keysem, NUM_SEMS, IPC_CREAT | 0666);
	/*if (sem_id == -1)
	{
		perror("#reader->semget");
		exit(EXIT_FAILURE);
	}
	*/
//======================================================================
//	Attaching shared memory segment
	char* shm = (char*) shmat(shm_id, NULL, 0);
	
	/*if (shm == (void*) -1){
		perror("#reader->shmat");
		exit(EXIT_FAILURE);
	}
	*/
//======================================================================
//	Synchronization
	
	sembuf sync[3];
	init_sop(sync[0], RD, 0, IPC_NOWAIT);
	init_sop(sync[1], RD, 2, SEM_UNDO);
	if (semop(sem_id, sync, 2) == -1){
		printf("Sorry, another reader already exists. Terminating..\n");
		exit(EXIT_SUCCESS);
	}
	
	init_sop(sync[0], WR, -1, 0);
	semop(sem_id, &sync[0], 1);
	
	//Semafore SHM has to be "zero" after terminating process
	init_sop(sync[0], SHM, 2, SEM_UNDO);
	init_sop(sync[1], SHM, -2, 0);
	semop(sem_id, sync, 2);
	
//======================================================================
//	Receving data
	
	int real_size = 1;	
	
	
	while (real_size > 0){
		sembuf sop[3];
		init_sop(sop[0], WR, -1, IPC_NOWAIT);
		init_sop(sop[1], WR, 1, 0);
		init_sop(sop[2], SHM, -1, 0);
		if (semop(sem_id, sop, 3) == -1){
			printf("#Sorry, writer was suddenly terminated. Terminating..\n");
			break;
		}
		
		real_size = scan_size(shm + SHM_SIZE);
		write(1, shm, real_size);

		semop(sem_id, &sop[2], 1);
	}
	/*
	sembuf sop[2];
	init_sop(sop[0], WR, -1, IPC_NOWAIT);
	init_sop(sop[1], WR, 1, 0);
		
	while(semop(sem_id, sop, 2) == 0){};
	*/
//======================================================================
//	Detaching shared memory segment
	/*if (shmdt(shm) == -1){
		perror("#reader->shmdt");
		exit(EXIT_FAILURE);
	}
	
	if (shmctl(shm_id, IPC_RMID, NULL) == -1){
		perror("#reader->shmctl");
		exit(EXIT_FAILURE);
	}

	if (semctl(sem_id, 0, IPC_RMID) == -1){
		perror("#reader->semctl");
		exit(EXIT_FAILURE);
	}
	*/
	return 0;
}

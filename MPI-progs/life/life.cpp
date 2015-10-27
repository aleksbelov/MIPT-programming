#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>
#include <unistd.h>
using namespace std;

struct str{
    char *now, *next;
};

class Field_t{
private:
    int SizeX, SizeY;
    str *data;
public:
    Field_t(){
        data = NULL;    
    }

    Field_t(int x, int y){
        SizeX = x;
        SizeY = y;
        data = new str[SizeY];
        for (int i = 0; i < SizeY; i++){
            data[i].now = new char[SizeX];
            data[i].next = new char[SizeX];
        }
        for (int i = 0; i < SizeY; i++)
            for (int j = 0; j < SizeX; j++)
                data[j].now[i] = '.';
    }
    ~Field_t(){
        if (data != NULL){
            for (int i = 0; i < SizeY; i++){
                delete [] data[i].now;
                delete [] data[i].next;
            }
            delete [] data;
        }
    }
    int getx(){
        return SizeX;
    }

    int gety(){
        return SizeY;
    }

    void step(int x, int y){
        int N = 0;
        for (int i = -1; i <=1; i++)
            for (int j = -1; j <=1; j++)
                if (i != 0 || j != 0){
                  //  if ((x + i >= 0) && (x + i < SizeX))
                  //      if ((y + j >= 0) && (y + j < SizeY)){
                            //printf("[%i/%i, %i/%i]='%c' ", x+i, SizeX, y+j, SizeY, data[y+j].now[x+i]);

                            if (data[(SizeY + y+j) % SizeY].now[(SizeX + x+i) % SizeX] == '#'){
                                N++;
                            }
                   //     }
                }
        
        if (data[y].now[x] == '.'){
            if (N == 3)
                data[y].next[x] = '#';
            else
                data[y].next[x] = '.';
        }
        else{
            if (N == 2 || N == 3)
                data[y].next[x] = '#';
            else
                data[y].next[x] = '.';
        
        }
    }
   
    void update(){
        for (int i = 0; i < SizeY; i++)
            memcpy(data[i].now, data[i].next, SizeX);
    }



    void init(char* filename){
        if (data == NULL){
            printf("The field is not allocated\n");
            return; 
        }
        ifstream fin;
        fin.open(filename);

        for (int i = 0; i < SizeY; i++){
            fin >> data[i].now;
            memcpy(data[i].next, data[i].now, SizeX);
        }
        fin.close();
    }

    void show(){
        if (data == NULL){
            printf("The field is not defined\n");
            return; 
        }
        
        system("clear");
        for (int i = 0; i < SizeY; i++){
            cout << data[i].now << endl;
            fflush(stdout);
        }
    }
        
    void show(char* filename){
        if (data == NULL){
            printf("The field is not defined\n");
            return; 
        }
        ofstream fout;
        fout.open(filename, ios_base::out);
        for (int i = 0; i < SizeY; i++)
            fout << data[i].now << endl;
        cout << endl; 
        fout.close();
    }

};


class semafor{
private:
    key_t key;
    int sem_id, N;   
public:
    semafor(int n){
        N = n;
        key = ftok(__FILE__, 1);
        sem_id = semget(key, N, IPC_PRIVATE | IPC_CREAT | 0666);
        semctl(sem_id, 0, SETVAL, 0);
    }

    ~semafor(){
        semctl(sem_id, 0, IPC_RMID);
    }
   
    void set_up(){
        finish_update();
    }

    void wait_and_run(int n){
        struct sembuf op[2];
        op[0].sem_num = n;
        op[0].sem_op = -1;
        op[0].sem_flg = 0;
        op[1].sem_num = n;
        op[1].sem_op = 1;
        op[1].sem_flg = 0;
        semop(sem_id, op, 2);
    }

    void finish_run(int n){
        struct sembuf op;
        op.sem_num = n;
        op.sem_op = -1;
        op.sem_flg = 0;
        semop(sem_id, &op, 1);
    }

    void wait_and_update(){
        struct sembuf* op = new struct sembuf [N];
        for (int i = 0; i < N; i++){
            op[i].sem_num = i;
            op[i].sem_op = 0;
            op[i].sem_flg = 0;
        }
        semop(sem_id, op, N);
    }

    void finish_update(){
        struct sembuf* op = new struct sembuf [N];
        for (int i = 0; i < N; i++){
            op[i].sem_num = i;
            op[i].sem_op = 1;
            op[i].sem_flg = 0;
        }
        semop(sem_id, op, N);
    }

    void print(){
        printf("Semafore value: ");
        for(int i = 0; i < N; i++)
            printf("%i ", semctl(sem_id, i, GETVAL));
        printf("\n");
        fflush(stdout);
    }
    void print(int n){
        printf("Semafore value: %i \n", semctl(sem_id, n, GETVAL));
        fflush(stdout);
    }


};

struct thread_args{
    Field_t* field;
    int Npthrds, SizeY, i;
    semafor* run;
};

#define ITER 50

void* thread_func(void* arg){
    thread_args* args = (thread_args*) arg;
    int startY = args->SizeY / args->Npthrds * args->i;
    int endY = args->SizeY;
    if (args->i != args->Npthrds - 1)
        endY = (args->SizeY / args->Npthrds) * (args->i+1);
    int r = args->SizeY - 1;
    for (int k = (args->SizeY % args->Npthrds - 1); k > 0; k--){
        if (args->i >= r)
            startY++;
        if ((args->i >= r-1) && (args->i != args->Npthrds - 1))
            endY++;
        r--;
    }
    
    //printf("%i: [%i, %i]\n", args->i, startY, endY);
    
    //for (;;){
    for (int k = 0; k < ITER; k++){
        args->run->wait_and_run(args->i);
        //printf("[%i] Enter cycle: ", args->i);
        //args->run->print(args->i);
        //fflush(stdout);
        //usleep(8000*args->i);
        for (int i = startY; i < endY; i++){
            for (int j = 0; j < args->field->getx(); j++){
                //printf("[%i, %i]: ", j, i);
                //cout << "Vasia-number = " << args->i << endl;
                args->field->step(j, i);
                //printf("\n");
            }
        }
        //printf("[%i]: %i\n", args->i, i);
        args->run->finish_run(args->i);
        
        //printf("[%i]  Exit cycle: ", args->i);
        //args->run->print(args->i);
        //fflush(stdout);
        
    }
    return NULL;
}


int main(int argc, char** argv){
    int SizeX = 50;
    int SizeY = 30;
    int Npthrds;
    Field_t field(SizeX, SizeY);  
    
    if (argc > 1){
        if (!strcmp(argv[1], "config")){
            field.show("config.txt");
            return 0;
        }
        else
            Npthrds = atoi(argv[1]);
    }
    else
        Npthrds = 1;
    
    semafor run(Npthrds);
    run.set_up();
    
    field.init("config.txt");

    thread_args args[Npthrds];
    pthread_t thread[Npthrds];

    for (int i = 0; i < Npthrds; i++){ 
        args[i].i = i;
        args[i].field = &field;
        args[i].Npthrds = Npthrds;
        args[i].SizeY = SizeY;
        args[i].run = &run;
        pthread_create(&thread[i], NULL, thread_func, (void*) &args[i]);
    }
    field.show();
    //for (;;){
    for(int k = 0; k < ITER-1; k++){    
        run.wait_and_update();
        
        field.update();
        field.show();
         
        run.finish_update();
        usleep(100000);
    }
    
    for (int i = 0; i < Npthrds; i++){
        void* res;
        pthread_join(thread[i], &res);
    }
    printf("VASYA!!!\n");
    return 0;
}

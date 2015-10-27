#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <iostream>

using std::cout;
using std::endl;

class Net {
private:
    double* data;
    int M, N;
public:
    Net (){
        
    }

    ~Net(){
        free(data);
    }

    void init(int m, int n){
        M = m;
        N = n;
        data = (double*) calloc(m*n, sizeof(double));
        //printf("Allocated %i*%i of double. Ptr %p\n", M, N, data);
        
        //for (int m = 0; m < M; m++){
        //    for (int n = 0; n < N; n++ )
        //        *(data+m*N + n) = (m+1)*(n+1);
        //}
    }

    void set(int m, int n, double x){
        *(data + N*m + n) = x;
    }

    
    double get(int m, int n){
        return *(data + N*m + n);
    }
    int getm(){
        return M;
    }
    int getn(){
        return N;
    }
    double* getdata(){
        return data;
    } 
    void adddata(double* d, int dataM){
        data =(double*) realloc(data, (M+dataM)*N*sizeof(double));
        for (int i = 0; i < dataM*N; i++)
            *(data + M*N + i) = d[N + i];
    }
    
    double* get_addr_for_add(int to_add){
        data = (double*)  realloc(data, (M + to_add - 1)*N*sizeof(double));
        //printf("         Vasya!!!  %i\n", res);
        double* ptr = (data + (M-1)*(N) );
        M += to_add - 1;
        return ptr;
    }
        

    void print(){
        for (int n = 0; n < N; n++){
            for (int m = 0; m < M; m++ )
                printf("%2lg ", this->get(m, n));
            printf("\n");
        }
    }

    void print_result(){
        printf("Result:\n");
        for (int m = 0; m < M; m++)
            printf("%2lg ", this->get(m, N-1));
        printf("\n");
    }
        

};

double f_right(int m, int n){
    return 0;
}

//double V(int m){
//    return 0;
//}


double scheme(int m, int n, double h, double t, class Net& data){
    //printf("(m,n): %lg\n", data.get(m,n));
    //printf("(m-1,n): %lg\n", data.get(m-1,n));
    return ( f_right(m, n) - (data.get(m, n) - data.get(m-1, n)) / h )*t + data.get(m, n);
}

// m - x, h
// n - t

int main(int argc, char **argv){
    

	int rank = 0, size = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    double xmin = 0, xmax = 0, XMIN = 0.0, XMAX = 200.0, tmin = 0.0, tmax = 10.0;
    double h = 0.01, t = 0.01;
    
    int Nx = (int)((XMAX - XMIN)/h);
    int Nt = (int)((tmax - tmin)/h);

    
    xmin = Nx / size * (rank);

    if (rank == size -1)
        xmax = Nx;
    else
        xmax = (Nx / size)*(rank + 1);
    int r = size-1;
    for (int i = (Nx % size-1); i > 0; i--){
        if (rank >= r)
            xmin++;
        if (rank >= r-1 && rank != size - 1)
            xmax++;
        r--;
    }

    printf("I am %i of %i, [%lg; %lg]\n", rank, size, xmin, xmax);
    

    Net tab;
    if (rank == 0){
        tab.init((xmax - xmin + 1),Nt);
        tab.set(0, 0, 1);
    }
    else
        tab.init((xmax - xmin + 1),Nt);
    
    for (int i = 0; i < tab.getm(); i++)
        tab.set(i, 0, 1);

    double starttime = 0, endtime = 0;
    //if (rank == 0)
        starttime = MPI_Wtime();

    for (int n = 0; n < tab.getn()-1; n++){ 
        /*if (rank == 0){
            printf("t = %i:\n", n);
            tab.print();
            printf("\n");
        }*/
        MPI_Status status;
        MPI_Request request;
        double buf = 0;
        if (n > 0 && rank > 0){
            MPI_Recv(&buf, 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &status );
            //printf("%i: t = %i recved: %lg\n", rank, n, buf);
            tab.set(0, n, buf);
        }
        for (int m = 1; m < tab.getm(); m++){
            tab.set(m, n +1, scheme(m, n, h, t, tab));
        }
        buf = tab.get(tab.getm()-1, n+1);
        if (rank < size-1){
            //printf("%i: send: %lg\n", rank, buf);
            MPI_Isend(&buf, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &request);
        }

    }
        endtime = MPI_Wtime();
    if (rank > 0){
        int M = tab.getm();
        //tab.print();
        MPI_Send(&M, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(tab.getdata(), tab.getn()*tab.getm(), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        //sleep(rank);
        //printf("To send from %i, dataM=%i:\n", rank, M);
        //tab.print();
    }
    if (rank == 0 & size > 1){
        MPI_Status status;
        int dataM = 0;
        //tab.print();
        for (int i = 1; i < size; i++){
            MPI_Recv(&dataM, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(tab.get_addr_for_add(dataM), tab.getn()*dataM, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
            //printf("After recv from %i: M=%i\n", i, tab.getm());
        }
        //tab.print();

    }

    //if (rank == 0)
        //endtime = MPI_Wtime();

    printf("Time [%i]: %lg c\n", rank, endtime-starttime);

    //sleep(size); 
    
    //if (rank == 0){
    //    printf("t = %i:\n", tab.getn()-1);
    //    tab.print();
    //}
    
    MPI_Finalize();
    return 0;
}

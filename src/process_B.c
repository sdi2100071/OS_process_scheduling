#define TEXT_SZ 2048
#define INITIAL_VALUE 0
#define KEY 12344

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>    
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h> 
#include <time.h>


/*SHARED MEMORY STRUCT*/
struct shared_use_st {
    sem_t semA;
    sem_t semB;
    clock_t start;
    clock_t end_time;
    char written_by_A[TEXT_SZ];
    char written_by_B[TEXT_SZ];
    char input[TEXT_SZ];
    float statistics[4][2];
    int first_piece;
    int whole_text;
    int iswritttingA;
    int iswritttingB;
    int pieces;
    int end;
};

int main(void){

    int main(void){

    //Get a shared memory segment 
    int shmid;  
    shmid = shmget((key_t)KEY, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    void* shared_memory = (void*)0;
    shared_memory = shmat(shmid, (void *)0, 0);
	
    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st *)shared_memory;

    //shared memory struct initialisation
    shared_data->end = 0; 


    //////////////*PROGRAM*////////////////////////

    sem_post(&shared_data->semA);   //unblocked A
    printf("I AM WAITING FOR PROCESS A TO UNBLOCK ME\n");
    
    sem_wait(&shared_data->semB);   //blocked B
    printf("PROCESS A UNBLOCKED ME I AM READY TO RUN\n");

}


}
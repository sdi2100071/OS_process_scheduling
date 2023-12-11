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
#include <sys/time.h>
#include "../../include/shared_memory.h"
#include "../../include/threadsA.h"


void* Allocate_shared_memory(int* id){

    /*ALLOCATION OF SHARED  MEMORY SEGMENT*/
    *id = shmget((key_t)KEY, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    void* shared_memory = (void*)0;
    shared_memory = shmat(*id, (void *)0, 0);

    /*ERROR CONTROL*/
    if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

    return shared_memory;
}

/*INITILISES ALL SHARED MEMORY'S STRUCT VARIABLES WITH 0*/

void Initialise_shared_mem(struct shared_use_st* shmem){

    struct shared_use_st* shared_data = shmem; 

    shared_data->end = 0; 
    shared_data->timeA = 0;
    shared_data->timeB = 0;
    shared_data->iswritttingA = 0;
    shared_data->iswritttingB = 0;
    shared_data->pieces = 0;
    shared_data->whole_text = 0;

    int i;
    for( i = 0; i < 3; i++ ){
        shared_data->statistics[i][0] = 0;
        shared_data->statistics[i][1] = 0;
    }

    /*semaphore initialisation*/
    int isshared = 1; // specifes whether a sem in shared (!= 0 -->not shared)
    sem_init(&shared_data->semA , isshared, INITIAL_VALUE);
    sem_init(&shared_data->semB , isshared, INITIAL_VALUE);

}

/*PRINTS ALL STATISTIC MESSAGES AT THE END OF EVERY PROCESSES CHAT*/

void Print_statistics(struct shared_use_st* shm , int p){

    struct shared_use_st *shared_data = shm;

    printf("\n------------STATISTICS------------\n");
   
    int mess_sent = 0;
    mess_sent = shared_data->statistics[0][p];
    printf("MESSAGES SENT: %d\n", mess_sent);

    printf("MESSAGES RECEIVED: %d\n", (int)shared_data->statistics[1][p]);

    printf("TOTAL PIECES: %d\n",(int)shared_data->statistics[2][p]);

    float avg = 0;
    float avg_time = 0;
    if(mess_sent){
        avg = (float)shared_data->statistics[2][p] / mess_sent;
        if(!p)
            avg_time = (float)(shared_data->timeA) / mess_sent;
        
        avg_time = (float)(shared_data->timeB) / mess_sent;

    }

    printf("PIECES PER MESSAGE: %0.2f\n" ,avg);
    printf("AVERAGE TIME: %0.3f microseconds\n" ,avg_time);
    
    printf("----------------------------------");
    printf("\n\n");

}


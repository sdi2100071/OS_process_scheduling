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




int main(){

    /*ALLOCATION OF SHARED  MEMORY SEGMENT*/
    int shmid;  //shared memory id 
    shmid = shmget((key_t)KEY, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    void* shared_memory = (void*)0;
    shared_memory = shmat(shmid, (void *)0, 0);
	
    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st *)shared_memory;
   
    /*INITIALISE SHARED MEM STRUCT */
    shared_data->end = 0; 
    shared_data->timeA = 0;
    shared_data->iswritttingA = 0;
    shared_data->iswritttingB = 0;
    shared_data->whole_text = 0;


    int i;
    for( i = 0; i <= 3; i++ ){
        shared_data->statistics[i][0] = 0;
        shared_data->statistics[i][1] = 0;
    }

    /*semaphore initialisation*/
  
    int isshared = 1; // specifes whether a sem in shared (!= 0 -->not shared)
    sem_init(&shared_data->semA , isshared, INITIAL_VALUE);
    sem_init(&shared_data->semB , isshared, INITIAL_VALUE);

    /////////////////CODE///////////////////////////////////////////

    // pthread_t thread_input;
    pthread_t thread_output;
    pthread_t thread_fgets;
    
    printf("Waitting for process B.....\n");
    sem_wait(&shared_data->semA);
    sem_post(&shared_data->semB);

    int res = 0;//elegxos gia error
    if(res){

    }
    res = pthread_create(&thread_output, NULL, thread_print,(void*)shared_data);
    res = pthread_create(&thread_fgets, NULL, thread_fget,(void*)shared_data);
    pthread_join(thread_output, NULL);


    printf("\nEND OF CHAT\n");
    
    printf("--------------------------------\n");
    int mess_sent = 0;
    mess_sent = shared_data->statistics[0][0];
    printf("MESSAGES SENT: %d\n", mess_sent);

    printf("MESSAGES RECEIVED: %d\n", (int)shared_data->statistics[1][0]);

    printf("TOTAL PIECES: %d\n",(int)shared_data->statistics[2][0]);

    float avg = 0;
    float avg_time = 0;
    if(mess_sent){
        avg = (float)shared_data->statistics[2][0] / mess_sent;
        avg_time = (float)(shared_data->timeA) / mess_sent;
    }
    printf("PIECES PER MESSAGE: %0.2f\n" ,avg);
    printf("AVERAGE TIME: %0.3f microseconds\n" ,avg_time);
    printf("--------------------------------\n");
    printf("\n\n"); 

    /*ERROR CONTROL*/
    if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

    if (shmdt(shared_memory) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}	

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}  


    return 0;
}
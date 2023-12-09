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
#include "../../include/threadsB.h"


int main(){

    /*ALLOCATION OF SHARED  MEMORY SEGMENT*/
    int shmid;  //shared memory id 
    shmid = shmget((key_t)KEY, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    void* shared_memory = (void*)0;
    shared_memory = shmat(shmid, (void *)0, 0);
	
    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st *)shared_memory;
    
    /////////////////CODE///////////////////////////////////////////

    // pthread_t thread_input;
    pthread_t thread_output;
    pthread_t thread_fgets;
    
    // parameters->thread = thread_input;

    sem_post(&shared_data->semA);
    sem_wait(&shared_data->semB);

    int res = 0;//elegxos gia error
    res = pthread_create(&thread_output, NULL, thread_print,(void*)shared_data);
    res = pthread_create(&thread_fgets, NULL, thread_fget,(void*)shared_data);
    pthread_join(thread_output, NULL);

    printf("\nEND OF CHAT\n");

    printf("--------------------------------\n");
     
    int mess_sent = 0;
    mess_sent = shared_data->statistics[0][1];

    printf("MESSAGES SENT: %d\n", mess_sent);
    printf("MESSAGES RECEIVED: %d\n", (int)shared_data->statistics[1][1]);

    printf("TOTAL PIECES: %d\n",(int)shared_data->statistics[2][1]);

    float avg = 0;
    float avg_time = 0.0;
    if(mess_sent){
        avg = (float)shared_data->statistics[2][1] / mess_sent;
        avg_time =  (float)(shared_data->timeB) / mess_sent;

    }
    printf("PIECES PER MESSAGE: %0.2f\n" ,avg);
    
    printf("AVERAGE TIME: %0.3f microseconds\n" ,avg_time);

    printf("--------------------------------\n");
    printf("\n\n"); 

    return 0;
}
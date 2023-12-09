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
#include "../../include/myfuncs.h"



int main(){

    /*ALLOCATION OF SHARED  MEMORY SEGMENT*/   
    int shmid;  //shared memory id 
    void* shared_memory = Allocate_shared_memory(&shmid);
    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st *)shared_memory;

    pthread_t thread_output;
    pthread_t thread_input;

    sem_post(&shared_data->semA);
    sem_wait(&shared_data->semB);

    int res = 0;//elegxos gia error
    res = pthread_create(&thread_output, NULL, thread_receive,(void*)shared_data);
    res = pthread_create(&thread_input, NULL, thread_send,(void*)shared_data);
    pthread_join(thread_output, NULL);

    printf("\nEND OF CHAT\n");

    int p = 1;
    Print_statistics(shared_data, p);

    return 0;
}
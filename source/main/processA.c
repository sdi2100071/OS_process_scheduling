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
#include "../../include/myfuncs.h"



int main(){

    /*ALLOCATION OF SHARED  MEMORY SEGMENT*/   
    int shmid;  //shared memory id 
    void* shared_memory = Allocate_shared_memory(&shmid);
    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st *)shared_memory;
   
    /*INITIALISE SHARED MEMORY STRUCT*/
    Initialise_shared_mem(shared_data);

    pthread_t thread_output;
    pthread_t thread_input;
    
    printf("Waitting for process B.....\n"); 
    sem_wait(&shared_data->semA);
    sem_post(&shared_data->semB);

    int res = 0;//elegxos gia error
    res = pthread_create(&thread_output, NULL, thread_receive,(void*)shared_data);
   
    res = pthread_create(&thread_input, NULL, thread_send,(void*)shared_data);
    pthread_join(thread_output, NULL);

    printf("\nEND OF CHAT\n");


    int p = 0;
    Print_statistics(shared_data, p);
        

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
#define TEXT_SZ 2048
#define INITIAL_VALUE 0
#define KEY 12375599

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

/*SHARED MEMORY STRUCT*/
struct shared_use_st {
    sem_t semA;
    sem_t semB;
    char written_by_A[TEXT_SZ];
    char written_by_B[TEXT_SZ];
    int iswritttingA;
    int iswritttingB;
    int end;
};

void* thread_read( void* shared_mem ){

    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st*) shared_mem;
    char buffer[BUFSIZ];
    shared_data->iswritttingA = 1;

    printf("Enter some text: ");   		
    fgets(buffer, BUFSIZ, stdin);
    strncpy(shared_data->written_by_A, buffer, TEXT_SZ);

    if (strncmp(buffer, "end", 3) == 0) {
        shared_data->end = 1;
        shared_data->iswritttingA = 0;
        return NULL;
    }
    // shared_data->iswritttingA = 0;

    return NULL;
}

void* thread_print( void* shared_mem ){

    return NULL;
}

  
int main(){

    int shmid;  //shared memory id 
    shmid = shmget((key_t)KEY, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    void* shared_memory = (void*)0;
    shared_memory = shmat(shmid, (void *)0, 0);
	
    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st *)shared_memory;
    shared_data->end = 0; 
    shared_data->iswritttingA = 0;
    shared_data->iswritttingB = 0;

    /*semaphore initialisation*/
  
    int isshared = 1; // specifes whether a sem in shared (!= 0 -->not shared)
    sem_init(&shared_data->semA , isshared, INITIAL_VALUE);
    sem_init(&shared_data->semB , isshared, INITIAL_VALUE);

    /*BLOCKED PROCCESS A*/
    printf("waiting for user...\n");

    int res; 
    pthread_t thread1;

    while(1){ 

        if(shared_data->end){
            sem_post(&shared_data->semB);
            break;
        }
        sem_wait(&shared_data->semA);
        sem_post(&shared_data->semB);
        
        shared_data->iswritttingA = 0;

        res = 0;
        res = pthread_create(&thread1, NULL, thread_read, (void*)shared_data);
        res = pthread_join(thread1, NULL);        

    }   


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

#define TEXT_SZ 2048
#define INITIAL_VALUE 0

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

struct shared_use_st {
    sem_t semA;
    sem_t semB;
    int end;
    char written_by_A[TEXT_SZ];
    char written_by_B[TEXT_SZ];
	
};

void* thread_send(void* out){

    struct shared_use_st *shared_data;
    shared_data =(struct shared_use_st*) out;

    char buffer[BUFSIZ]; //array of input
    printf("Enter some text: ");
    		
    fgets(buffer, BUFSIZ, stdin);

    strncpy(shared_data->written_by_A, buffer, TEXT_SZ);

    if (strncmp(buffer, "end", 3) == 0) {
        shared_data->end = 1;
    }

    return NULL;
}

void* thread_get(void* input ){
    printf("LOL");
    return NULL;
}

  
int main(){

    int running = 1;
    struct shared_use_st *shared_data;
    char buffer[BUFSIZ]; //array of input
    int shmid;  //shared memory id 
    int isshared = 1; // specifes whether a sem in shared (!=0 -->not shared)
    int sem_val; 
    int res = 0; //thread's return

    
    shmid = shmget((key_t)12375599, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    void* shared_memory = (void*)0;
    shared_memory = shmat(shmid, (void *)0, 0);
	
    if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	
    printf("Shared memory segment with id %d attached at %p\n", shmid, shared_memory);
    shared_data = (struct shared_use_st *)shared_memory;
    shared_data->end = 0;

    /*semaphore initialisation*/
    sem_init(&shared_data->semA , isshared, INITIAL_VALUE);
    sem_init(&shared_data->semB , isshared, INITIAL_VALUE);

    pthread_t thread1;

    /*BLOCKED PROCCESS A*/
    sem_wait(&shared_data->semA);

    while(running){  
        
        res = 0;
        if(shared_data->end){
            break;
        }
        printf("waiting for user...\n");

        res = pthread_create(&thread1, NULL, thread_send, (void*)shared_data);
        res = pthread_join(thread1, NULL); 
      
        if (shared_data->end == 1) {
            running = 0;
            printf("-------------");
            break;
        }

    }
    
    if(!shared_data->end){
        sem_post(&shared_data->semB);
    }
    //sem_post(&shared_data->semB);

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

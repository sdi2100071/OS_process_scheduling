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

struct shared_use_st {
    sem_t semA;
    sem_t semB;
    int end;
	char some_text[TEXT_SZ];
};

int main(){

    int running = 1;
    void* shared_memory = (void*)0;
    struct shared_use_st *shared_data;
    char buffer[BUFSIZ]; //array of input
    int shmid;  //shared memory id 
    shmid = shmget((key_t)123755, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    int isshared = 1; // specifes whether a sem in shared (!=0 -->not shared)

    shared_memory = shmat(shmid, (void *)0, 0);
	if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

	printf("Shared memory segment with id %d attached at %p\n", shmid, shared_memory);
    shared_data = (struct shared_use_st *)shared_memory;

    shared_data->end = 0;

    
    int sem_val ; 
    sem_getvalue(&shared_data->semB , &sem_val );
    printf("semB = %d\n" , sem_val );

    sem_getvalue(&shared_data->semA , &sem_val );
    printf("semA = %d\n" , sem_val ); 
    
    sem_post(&shared_data->semA); 
    while(running){

        if(shared_data->end){
            break;
        }

        printf("waiting for user...\n");
        printf("Enter some text: ");		
        fgets(buffer, BUFSIZ, stdin);
        strncpy(shared_data->some_text, buffer, TEXT_SZ);
        if (strncmp(buffer, "end", 3) == 0) {
            running = 0;
            shared_data->end = 1;
            break;
        }
    }
    
    if(!shared_data->end){
    sem_wait(&shared_data->semB);
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

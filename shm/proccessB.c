#define TEXT_SZ 2048
#define INITIAL_VALUE 0
#define KEY 1237559

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
    char written_by_A[TEXT_SZ];
    char written_by_B[TEXT_SZ];
    int whole_text;
    int iswritttingA;
    int iswritttingB;
    int pieces;
    int end;
};

void* thread_read(void* shared_mem){

    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st*) shared_mem;
    char buffer[BUFSIZ];
    int input_size;

    if(shared_data->end == 1 ){
        return NULL;
    }

    printf("Enter some text for B: ");		
    fgets(buffer, BUFSIZ, stdin);
    input_size =strlen(buffer);
    shared_data->pieces = input_size / 15;
    if( input_size % 15 ){
        shared_data->pieces ++;
    }
    int i;
    for( i = 0; i < shared_data->pieces; i++){
        strncpy(&shared_data->written_by_B[i*15], &buffer[i*15], 15);
    }
    shared_data->whole_text = 1;
    printf("\n");
    
    shared_data->iswritttingB = 1;

    if (strncmp(buffer, "end", 3) == 0) {
        shared_data->end = 1;
        return NULL;
    }
    return NULL;
}

void* thread_print(void* shared_mem){

    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st*) shared_mem;
    char buffer[BUFSIZ];

    if(shared_data->end ){
        return NULL;
    }
    printf("\n");
    printf("A WROTE: ");

    while(!shared_data->whole_text);

    printf("%s\n",shared_data->written_by_A);

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

    printf("waiting for user...\n");

    int res;
    pthread_t thread1;
    pthread_t thread2;
    while(1){

        if(shared_data->end == 1){
            pthread_cancel(thread1);
            break;
        }

        sem_post(&shared_data->semA); 
        sem_wait(&shared_data->semB);
        
        res = 0;
        res = pthread_create(&thread1, NULL, thread_read, (void*)shared_data);   

        while(!shared_data->iswritttingA && !shared_data->iswritttingB);

        if(shared_data->iswritttingA && !shared_data->end){
            printf("\n");
            pthread_cancel(thread1);
            res = pthread_create(&thread2, NULL, thread_print, (void*)shared_data);
        }
        
        res = pthread_join(thread1, NULL); 
        shared_data->iswritttingB = 0;

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
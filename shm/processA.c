#define TEXT_SZ 2048
#define INITIAL_VALUE 0
#define KEY 123446

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
    float statistics[4][2];
    int first_piece;
    int whole_text;
    int iswritttingA;
    int iswritttingB;
    int pieces;
    int end;
};


struct thread_parameters{

    struct shared_use_st *shared_mem;
    pthread_t thread;

};

void* thread_fget(void* parameter){

    struct thread_parameters *param;
    struct shared_use_st *shared_data;
    param = (struct thread_parameters*)parameter;
    shared_data = param->shared_mem;
    char buffer[TEXT_SZ];
    int input_size;

    while(1){

        printf("Enter some text for A:\n ");   		
        fgets(buffer, BUFSIZ, stdin);
        shared_data->iswritttingA = 1;
        
        input_size = strlen(buffer);
        shared_data->pieces = input_size / 15;
        if( input_size % 15 ){
            shared_data->pieces ++;
        }

        shared_data->statistics[2][0] += shared_data->pieces;
    	
        int i;
        for(i = 0; i < shared_data->pieces; i++){
            if(i == 0){
                shared_data->first_piece = 1;
                // shared_data->start = clock();
            }
        else{
            shared_data->first_piece = 0;
            }       
            strncpy(&shared_data->written_by_A[i*15], &buffer[i*15], 15);
        }
        shared_data->statistics[0][0]++;
        shared_data->whole_text = 1;

        if (strncmp(buffer, "end", 3) == 0) {
            shared_data->end = 1;
            return NULL;
        }

    }

    return NULL;
}

void* thread_print(void* parameter){

    struct thread_parameters *param;
    struct shared_use_st *shared_data;
    param = (struct thread_parameters*)parameter;
    shared_data = param->shared_mem;

    while(1){
        
        if(shared_data->end)
            return NULL;

        if(shared_data->iswritttingB){
            while(!shared_data->whole_text);
            shared_data->statistics[1][0]++;
            printf("\nB WROTE:%s\n",shared_data->written_by_B);
            shared_data->iswritttingB = 0;
        }

    }

    return NULL;
}

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
    shared_data->iswritttingA = 0;
    shared_data->iswritttingB = 0;
    int whole_text = 0;

    struct thread_parameters *parameters;

    parameters->shared_mem = shared_data;

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
    res = pthread_create(&thread_output, NULL, thread_print,(void*)parameters);
    res = pthread_create(&thread_fgets, NULL, thread_fget,(void*)parameters);
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
    }
    printf("PIECES PER MESSAGE: %f\n" ,avg);
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
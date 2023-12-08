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
        printf("Enter some text for B:\n ");   		
        fgets(buffer, BUFSIZ, stdin);
        
        input_size = strlen(buffer);
        shared_data->pieces = input_size / 15;
        if( input_size % 15 ){
            shared_data->pieces ++;
        }

        shared_data->statistics[2][1] += shared_data->pieces;
    	
        int i;
        for(i = 0; i < shared_data->pieces; i++){
            if(i == 0){
                shared_data->first_piece = 1;
                shared_data->start = clock();
            }
        else{
            shared_data->first_piece = 0;
        }       
            strncpy(&shared_data->written_by_B[i*15], &buffer[i*15], 15);
        }

        shared_data->statistics[0][1]++;
        shared_data->whole_text = 1;
        shared_data->iswritttingB = 1;
   
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
            break;

        if(shared_data->iswritttingA){
            while(!shared_data->whole_text);
            printf("\nA WROTE: %s\n", shared_data->written_by_A);
            shared_data->iswritttingA = 0;
            shared_data->statistics[1][1]++;
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

    struct thread_parameters *parameters;
    parameters->shared_mem = shared_data;

    /////////////////CODE///////////////////////////////////////////

    // pthread_t thread_input;
    pthread_t thread_output;
    pthread_t thread_fgets;
    
    // parameters->thread = thread_input;

    sem_post(&shared_data->semA);
    sem_wait(&shared_data->semB);

    int res = 0;//elegxos gia error
    res = pthread_create(&thread_output, NULL, thread_print,(void*)parameters);
    res = pthread_create(&thread_fgets, NULL, thread_fget,(void*)parameters);
    pthread_join(thread_output, NULL);

    printf("\nEND OF CHAT\n");

    printf("--------------------------------\n");
     
    int mess_sent = 0;
    mess_sent = shared_data->statistics[0][1];

    printf("MESSAGES SENT: %d\n", mess_sent);
    printf("MESSAGES RECEIVED: %d\n", (int)shared_data->statistics[1][1]);

    printf("TOTAL PIECES: %d\n",(int)shared_data->statistics[2][1]);

    float avg = 0;
    float avg_time = 0;
    if(mess_sent){
        avg = (float)shared_data->statistics[2][1] / mess_sent;
    }
    printf("PIECES PER MESSAGE: %f\n" ,avg);
    printf("--------------------------------\n");
    printf("\n\n"); 

    return 0;
}    
#define TEXT_SZ 2048
#define INITIAL_VALUE 0
#define KEY 123445

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
    char input[TEXT_SZ];
    float statistics[4][2];
    int first_piece;
    int whole_text;
    int iswritttingA;
    int iswritttingB;
    int pieces;
    int end;
};


void* thread_string(void* share_mem){
    char outp[TEXT_SZ];
    struct shared_use_st * shared_data;
    shared_data = share_mem;
    printf("\nEnter some text for A: ");   		
    fgets((char*)outp, TEXT_SZ, stdin);
    strncpy(shared_data->input, outp, TEXT_SZ);
    shared_data->iswritttingA = 1;
}


void* thread_read( void* shared_mem ){

    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st*) shared_mem;
    char buffer[TEXT_SZ];
    int input_size;
    int res = 0;
    pthread_t inputA;


    while(1){

        sem_wait(&shared_data->semA);
        sem_post(&shared_data->semB);

        shared_data->iswritttingA = 0;
        shared_data->iswritttingB = 0;
          
        if(shared_data->end == 1 )
            return NULL;

        res = pthread_create(&inputA, NULL, thread_string, (void*)shared_data);   

        while(shared_data->iswritttingA == 0 && shared_data->iswritttingB == 0);

        if(shared_data->iswritttingB)
            pthread_cancel(inputA);

        pthread_join(inputA, NULL);

       
            // sem_wait(&shared_data->semA);
            // sem_post(&shared_data->semB);
        

        if(shared_data->iswritttingA){
            strncpy(buffer, shared_data->input, TEXT_SZ);
            input_size = strlen(shared_data->input);
            shared_data->pieces = input_size / 15;
            if( input_size % 15 ){
                shared_data->pieces ++;
            }

            shared_data->statistics[2][0] += shared_data->pieces;
            
            int i;
            for( i = 0; i < shared_data->pieces; i++){
                if(i == 0){
                    shared_data->first_piece = 1;
                    shared_data->start = clock();
                }
                else{
                    shared_data->first_piece = 0;
                }
                
                strncpy(&shared_data->written_by_A[i*15], &buffer[i*15], 15);
            }

            shared_data->whole_text = 1;

            if (strncmp(buffer, "end", 3) == 0) {
                shared_data->end = 1;
            }
        }

        sem_wait(&shared_data->semA);
        sem_post(&shared_data->semB);

        if(shared_data->iswritttingB){

            printf("\nB WROTE: ");

            if(shared_data->first_piece){
                shared_data->end_time = clock();
                shared_data->statistics[3][1] += ((double)(shared_data->end_time - shared_data->start)) /  CLOCKS_PER_SEC;
            }

            while(!shared_data->whole_text);

            printf("%s",shared_data->written_by_B);
        }

    }
    return NULL;
}

void* thread_print( void* shared_mem ){

    struct shared_use_st *shared_data;
    shared_data = (struct shared_use_st*) shared_mem;
    char buffer[BUFSIZ];

    shared_data->statistics[1][0]++;

    if(shared_data->end ){
        return NULL;
    }
    // printf("\n");
    printf("B WROTE: ");

    if(shared_data->first_piece){
      
       shared_data->end_time = clock();
       shared_data->statistics[3][1] += ((double)(shared_data->end_time - shared_data->start)) /  CLOCKS_PER_SEC;
    //    long double tm = ((long double)(shared_data->end_time - shared_data->start)) /  CLOCKS_PER_SEC;
    //    printf("time = %Lf\n", tm);

    }

    while(!shared_data->whole_text);

    printf("%s\n",shared_data->written_by_B);

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
    int whole_text = 0;

    int i;
    for( i = 0; i <= 3; i++ ){
        shared_data->statistics[i][0] = 0;
        shared_data->statistics[i][1] = 0;
    }

 
    /*semaphore initialisation*/
  
    int isshared = 1; // specifes whether a sem in shared (!= 0 -->not shared)
    sem_init(&shared_data->semA , isshared, INITIAL_VALUE);
    sem_init(&shared_data->semB , isshared, INITIAL_VALUE);


    /*BLOCKED PROCCESS A*/
    printf("waiting for process B to unblock me...\n");

    int res; 
    pthread_t thread1;


    sem_wait(&shared_data->semA); //A BLOCKED 
    sem_post(&shared_data->semB); //POST B 

    res = pthread_create(&thread1, NULL, thread_read, (void*)shared_data);
    res = pthread_join(thread1, NULL);

    printf("--------------------------------\n");

    int mess_sent = 0;
    mess_sent = shared_data->statistics[0][0];
    printf("MESSAGES I SENT: %d\n", mess_sent);

    printf("MESSAGES I GOT: %d\n", (int)shared_data->statistics[1][0]);

    printf("TOTAL PIECES: %d\n",(int)shared_data->statistics[2][0]);

    
    float avg = 0;
    float avg_time = 0;
    if(mess_sent){
        avg = (float)shared_data->statistics[2][0] / mess_sent;
        float avg_time = shared_data->statistics[3][1] / mess_sent;
    }
    printf("PIECES PER MESSAGE: %f\n" ,avg );

    avg_time = shared_data->statistics[3][1] ;
    printf("AVERAGE TIME = %f seconds\n",avg_time);

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
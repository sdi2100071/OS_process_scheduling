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


void* thread_fget(void* parameter){

    struct shared_use_st *shared_data;
    shared_data =( struct shared_use_st*)parameter;
    struct timeval current_time;
    
    char buffer[TEXT_SZ];
    int input_size;

    while(1){  
        
        printf("Enter some text for A:\n "); 

        gettimeofday(&current_time,NULL);
        shared_data->start = current_time.tv_sec;

        fgets(buffer, BUFSIZ, stdin);
        shared_data->iswritttingA = 1;
        
        input_size = strlen(buffer);
        shared_data->pieces = input_size / 15;
        if( input_size % 15 )
            shared_data->pieces ++;      
        shared_data->statistics[2][0] += shared_data->pieces;
    	
        int i;
        for(i = 0; i < shared_data->pieces; i++){
            if(i == 0)             
                shared_data->first_piece = 1;      
            else
                shared_data->first_piece = 0;
                   
            strncpy(&shared_data->written_by_A[i*15], &buffer[i*15], 15);
        }


        shared_data->statistics[0][0]++;
        shared_data->whole_text = 1;
        sem_wait(&shared_data->semA);

        if (strncmp(buffer, "end", 3) == 0) {
            shared_data->end = 1;
            return NULL;
        }
    }

    return NULL;
}


void* thread_print(void* parameter){
    
    struct shared_use_st *shared_data;
    shared_data =( struct shared_use_st*)parameter;
    struct timeval endoftime;

    while(1){
        
        if(shared_data->end)
            return NULL;

        if(shared_data->iswritttingB){
            
            if(shared_data->first_piece){
                gettimeofday(&endoftime,NULL);
                shared_data->timeB += endoftime.tv_sec - shared_data->start;
            }
            
            while(!shared_data->whole_text);
            printf("\nB WROTE:%s\n",shared_data->written_by_B);

            sem_post(&shared_data->semB);

            shared_data->statistics[1][0]++;            
            shared_data->iswritttingB = 0;
        }

    }

    return NULL;
}
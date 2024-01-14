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



void* thread_send(void* parameter){

    struct shared_use_st *shared_data;
    shared_data =( struct shared_use_st*)parameter;
    struct timeval current_time;

    char buffer[TEXT_SZ];
    int input_size;

    while(1){

        printf("Enter some text for B:\n ");  

        /*keeps in shared_data the moment it starts waitting for input each time*/
        gettimeofday(&current_time,NULL);
        shared_data->start = current_time.tv_sec; 	

        fgets(buffer, BUFSIZ, stdin);
        
        //counts pieces of each message
        input_size = strlen(buffer);
        shared_data->pieces = input_size / 15;
        if( input_size % 15 ){
            shared_data->pieces ++;
        }
        shared_data->statistics[2][1] += shared_data->pieces;   //sum of pieces
    	
        /*sends message to the written_by_B buffer in blocks of 15*/
        int i;
        for(i = 0; i < shared_data->pieces; i++){
            if(i == 0)
                shared_data->first_piece = 1;
            
            else
                shared_data->first_piece = 0;
             
            strncpy(&shared_data->written_by_B[i*15], &buffer[i*15], 15);
        }

        shared_data->statistics[0][1]++;    //counter of messages B sent
        shared_data->whole_text = 1;    //whole text delivered
        shared_data->iswritttingB = 1;

        /*B blocked until A prints message*/
        sem_wait(&shared_data->semB);

        /*If B wrote #BYE# -> end of chat*/
        if (strncmp(buffer, "#BYE#", 5) == 0) {
            shared_data->end = 1; 
            return NULL;
        }

    }

    return NULL;
}



void* thread_receive(void* parameter ){

    struct shared_use_st *shared_data;
    shared_data =( struct shared_use_st*)parameter;
    struct timeval endoftime;

    while(1){

        /*If someone send #BYE# -> end of chat*/
        if(shared_data->end)
            break;

        if(shared_data->iswritttingA){

            /*if the first piece was delivered keep the exact moment it arrived*/
            if(shared_data->first_piece){
                gettimeofday(&endoftime,NULL);
                shared_data->timeA += endoftime.tv_sec - shared_data->start;
            }
            
            //wait for the whole message to be delivered 
            while(!shared_data->whole_text);
            printf("\nA WROTE: %s\n", shared_data->written_by_A);
            
            //unblock A
            shared_data->statistics[1][1]++;    //counter of messages received
            sem_post(&shared_data->semA);

            shared_data->iswritttingA = 0;

        }   

    }

    return NULL;
}

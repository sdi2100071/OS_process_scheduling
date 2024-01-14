#define TEXT_SZ 2048
#define INITIAL_VALUE 0
#define KEY 1234477

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*SHARED MEMORY STRUCT*/
struct shared_use_st {
    sem_t semA;
    sem_t semB;
    long start;
    long timeA;     //sum 
    long timeB;     //sum
    char written_by_A[TEXT_SZ];     //mess buffer of A
    char written_by_B[TEXT_SZ];     //mess buffer of B
    float statistics[3][2];
    int first_piece;
    int whole_text;
    int iswritttingA;
    int iswritttingB;
    int pieces;
    int end;
};



//////                                                 [0]      [1]       
//////statistics[3][2]:                              [procA]  [procB]
/////                     [0] [messages sent]              
/////                     [1] [messages received]          
/////                     [2] [total pieces sent]          
/////                       
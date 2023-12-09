#define TEXT_SZ 2048
#define INITIAL_VALUE 0
#define KEY 12344

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*SHARED MEMORY STRUCT*/
struct shared_use_st {
    sem_t semA;
    sem_t semB;
    long start;
    long timeA;
    long timeB;
    char written_by_A[TEXT_SZ];
    char written_by_B[TEXT_SZ];
    float statistics[3][2];
    int first_piece;
    int whole_text;
    int iswritttingA;
    int iswritttingB;
    int pieces;
    int end;
};
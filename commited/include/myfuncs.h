
/*Allocates a shared memory segment*/
/*Returns a void pointer to the shared memory and the shared mem id at the argument int* */
void* Allocate_shared_memory(int*);


/*Initialises with zero the shared memory variables of the struct named shared_data*/
/*and the semaphores used*/
void Initialise_shared_mem(struct shared_use_st* );


/*Prints statistic messages of each process*/
/*The second argument of the function (int) refers to whether it procA(=0) or procB(=1)*/
void Print_statistics(struct shared_use_st*, int );
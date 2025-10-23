
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "kernel.h"
#include "task.h"

typedef struct semaphore
{
    int count ; // 1 biến đếm 
    tcb_t* task_waiting_list ; 
}semaphore_t;


void semaphore_init(semaphore_t* sem , int initial );
// int initial la gia tri khoi tao semaphore
void semaphore_wait(semaphore_t* sem);
void semaphore_signal(semaphore_t* sem);












#endif

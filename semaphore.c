#include "semaphore.h"
#include "kernel.h"
#include "task.h"
#include "stdlib.h"
static inline void __disable_irq(void) {
    __asm volatile ("cpsid i" : : : "memory");
}

static inline void __enable_irq(void) {
    __asm volatile ("cpsie i" : : : "memory");
}
extern tcb_t* current_task;


void semaphore_init(semaphore_t* sem, int initial)
{
        sem->count  = initial ; 
        sem->task_waiting_list =   NULL ; 
}

void semaphore_wait(semaphore_t* sem)
{
   __disable_irq();
   sem->count-- ; 
   if(sem->count < 0)
   {
    current_task->state = TASK_BLOCKED ; 
   
   if(!sem->task_waiting_list)
   {
    sem->task_waiting_list = current_task ; 
    current_task->next = current_task ; 
   }else{
    tcb_t* t = sem->task_waiting_list ; 
    while(t->next != sem->task_waiting_list)
    {
        t = t->next ;}
    t->next = current_task ; 
    current_task->next = sem->task_waiting_list ;      
   }
   __enable_irq();

   ICSR |= PENDSVSET_BIT;
   while(current_task->state == TASK_BLOCKED);
   return ; 
    }
}

void semaphore_signal(semaphore_t* sem)
{
    __disable_irq();
    sem->count++ ; 

    // code danh thuc 
    if(sem->count <= 0 && sem->task_waiting_list)
    {
        tcb_t* task_wake = sem->task_waiting_list ; 
        // xóa task đầu tiên khỏi danh sách chờ 
        if(task_wake->next == task_wake) sem->task_waiting_list = NULL;
        else{
            tcb_t* t = sem->task_waiting_list ; 
            while(t->next != sem->task_waiting_list)
            {
                t = t->next;
                t->next = task_wake->next ; 
                sem->task_waiting_list = task_wake->next ; 

            }
            task_wake->state = TASK_READY;
            ICSR |= PENDSVSET_BIT;
        }
        __enable_irq();

    }



}

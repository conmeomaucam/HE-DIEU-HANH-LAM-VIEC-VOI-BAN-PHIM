#include "task.h"
#include "heap.h"
#include "kernel.h"

// Dinh nghia ICSR de co the kich hoat PendSV
#define ICSR            (*(volatile uint32_t *)0xE000ED04)
#define PENDSVSET_BIT   (1 << 28)




// Khai bao bien current_task tu kernel.c
extern tcb_t* current_task;
static uint32_t next_task_id = 0;

tcb_t* task_create(
    void (*task_function)(void),
    const char* name,
    uint8_t priority,
    uint32_t stack_size
) {
    tcb_t* new_tcb = (tcb_t*)malloc(sizeof(tcb_t));
    if (!new_tcb) return 0;

    // --- Khoi tao cac truong moi ---
    new_tcb->task_id = next_task_id++;
    new_tcb->task_name = name;
    new_tcb->priority = priority;
    new_tcb->state = TASK_READY; // Tac vu moi luon o trang thai san sang
    new_tcb->sleep_until_tick = 0;
    
    uint8_t* stack = (uint8_t*)malloc(stack_size);
    if (!stack) {
        free(new_tcb);
        return 0;
    }
    
    new_tcb->stack_base = stack;
    new_tcb->stack_size = stack_size;

    uint32_t* stack_top = (uint32_t*)(stack + stack_size);
    
    // Phan dan dung ngan xep giu nguyen
    stack_top[-1] = 0x01000000;
    stack_top[-2] = (uint32_t)task_function;
    stack_top[-3] = 0xFFFFFFFD;
    for(int i = 4; i <= 16; i++) { stack_top[-i] = 0; }
    
    new_tcb->stack_ptr = (void*)(&stack_top[-16]);
    return new_tcb;
}

void task_delay(uint32_t ms) {
    if (current_task) {
        // Dat bao thuc
        uint32_t current_ticks = kernel_get_ticks();
        current_task->sleep_until_tick = current_ticks + ms;
        // Chuyen trang thai sang "dang ngu"
        current_task->state = TASK_SLEEPING;
        // Chu dong yeu cau chuyen doi ngu canh
        ICSR |= PENDSVSET_BIT;
    }




 
}

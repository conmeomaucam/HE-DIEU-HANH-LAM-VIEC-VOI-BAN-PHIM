// task.c
#include <stdint.h>
#include <stddef.h>
#include "task.h"
#include "heap.h"
#include "kernel.h"

#define ICSR            (*(volatile uint32_t *)0xE000ED04)
#define PENDSVSET_BIT   (1u << 28)

extern tcb_t* current_task;
static uint32_t next_task_id = 0;

tcb_t* task_create(void (*task_function)(void),
                   const char* name,
                   uint8_t priority,
                   uint32_t stack_size)
{
    tcb_t* new_tcb = (tcb_t*)malloc(sizeof(tcb_t));
    if (!new_tcb) return 0;

    new_tcb->task_id = next_task_id++;
    new_tcb->task_name = name;
    new_tcb->priority = priority;
    new_tcb->state = TASK_READY;
    new_tcb->sleep_until_tick = 0;
    new_tcb->next = NULL;

    uint8_t* stack = (uint8_t*)malloc(stack_size);
    if (!stack) { free(new_tcb); return 0; }
    new_tcb->stack_base = stack;
    new_tcb->stack_size = stack_size;

    // PSP bắt đầu ở đỉnh stack, căn 8 byte (rất quan trọng trên ARMv7-M)
    uint32_t* stk = (uint32_t*)(stack + stack_size);
    stk = (uint32_t*)(((uintptr_t)stk) & ~0x7u);

    // --- Hardware-saved context ---
    *(--stk) = 0x01000000;                  // xPSR (Thumb bit set)
    *(--stk) = (uint32_t)task_function;     // PC
    *(--stk) = 0xFFFFFFFD;                  // LR (EXC_RETURN -> Thread/PSP)
    *(--stk) = 0x12121212;                  // R12
    *(--stk) = 0x03030303;                  // R3
    *(--stk) = 0x02020202;                  // R2
    *(--stk) = 0x01010101;                  // R1
    *(--stk) = 0x00000000;                  // R0

    // --- Software-saved context (r4–r11) ---
    for (int i = 0; i < 8; i++) *(--stk) = 0;

    new_tcb->stack_ptr = (void*)stk;        // PSP initial
    return new_tcb;
}

void task_delay(uint32_t ms) {
    if (current_task) {
        uint32_t now = kernel_get_ticks();
        current_task->sleep_until_tick = now + ms;
        current_task->state = TASK_SLEEPING;
        ICSR |= PENDSVSET_BIT;              // yêu cầu switch
    }
}


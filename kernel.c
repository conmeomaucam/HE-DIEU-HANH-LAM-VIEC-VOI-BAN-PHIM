// kernel.c
#include <stdint.h>
#include <stddef.h>
#include "kernel.h"
#include "heap.h"
#include "task.h"
#include "uart.h"

#define SYSTICK_BASE    0xE000E010
#define STK_CTRL        (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define STK_LOAD        (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define ICSR            (*(volatile uint32_t *)0xE000ED04)
#define PENDSVSET_BIT   (1u << 28)

#define CPU_CLOCK_HZ    12000000u

static volatile uint32_t system_ticks = 0;

tcb_t* taskC = 0;          // để UART ISR đánh thức
tcb_t* current_task = 0;
tcb_t* task_list_head = 0;

void SysTick_Handler(void) {
    system_ticks++;

    // đánh thức task hết ngủ
    if (task_list_head) {
        tcb_t* t = task_list_head;
        do {
            if (t->state == TASK_SLEEPING && system_ticks >= t->sleep_until_tick) {
                t->state = TASK_READY;
            }
            t = t->next;
        } while (t != task_list_head);
    }
    ICSR |= PENDSVSET_BIT;   // yêu cầu schedule
}

void scheduler_priority_based(void) {
    if (!task_list_head) return;

    tcb_t* t = task_list_head;
    tcb_t* next = NULL;
    uint8_t maxp = 0;

    do {
        if (t->state == TASK_READY && t->priority >= maxp) {
            maxp = t->priority;
            next = t;
        }
        t = t->next;
    } while (t != task_list_head);

    if (next) {
        if (current_task && current_task->state == TASK_RUNNING)
            current_task->state = TASK_READY;
        current_task = next;
        current_task->state = TASK_RUNNING;
        // (tuỳ chọn) debug ngoài handler
        // uart_puts("[Scheduler] -> task "); uart_puti(current_task->task_id); uart_puts("\n");
    }
}

void kernel_init(void) { heap_init(); }
uint32_t kernel_get_ticks(void) { return system_ticks; }

void kernel_add_task(tcb_t* nt) {
    if (!task_list_head) {
        task_list_head = nt;
        nt->next = nt;
    } else {
        tcb_t* c = task_list_head;
        while (c->next != task_list_head) c = c->next;
        c->next = nt;
        nt->next = task_list_head;
    }
}

void __attribute__((naked)) kernel_launch(void) {
    current_task = task_list_head;
    if (current_task) current_task->state = TASK_RUNNING;

    // 1ms tick
    STK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1u;
    STK_CTRL = (1u<<2) | (1u<<1) | (1u<<0);   // CLKSOURCE | TICKINT | ENABLE

    __asm volatile("svc 0");  // vào Thread mode dùng PSP theo SVC_Handler
}

/* Cho task monitor duyệt */
const tcb_t* kernel_get_task_list_head(void) { return task_list_head; }

/* !!! KHÔNG định nghĩa PendSV_Handler ở đây nữa (dùng bản ASM trong startup.s) */


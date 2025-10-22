#ifndef KERNEL_H
#define KERNEL_H
#include <stdint.h>
#include "task.h"
#define SYSTICK_BASE    0xE000E010
#define STK_CTRL        (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define STK_LOAD        (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define ICSR            (*(volatile uint32_t *)0xE000ED04)
#define PENDSVSET_BIT   (1 << 28)

#define CPU_CLOCK_HZ    12000000
#define TICK_RATE_HZ    1000 // Tang toc SysTick len 1000Hz (1ms moi nhip)
#define SYSTICK_BASE    0xE000E010
#define STK_CTRL        (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define STK_LOAD        (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define ICSR            (*(volatile uint32_t *)0xE000ED04)
#define PENDSVSET_BIT   (1 << 28)

#define CPU_CLOCK_HZ    12000000
#define TICK_RATE_HZ    10 // Tang toc SysTick len 1000Hz (1ms moi nhip)
void PendSV_Handler(void);
void kernel_init(void);
void kernel_add_task(tcb_t* new_task);
void kernel_launch(void);
uint32_t kernel_get_ticks(void);
const tcb_t* kernel_get_task_list_head(void);
void heap_init(void);
#endif

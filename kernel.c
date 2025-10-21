#define SYSTICK_BASE    0xE000E010
#define STK_CTRL        (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define STK_LOAD        (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define ICSR            (*(volatile uint32_t *)0xE000ED04)
#define PENDSVSET_BIT   (1 << 28)

#define CPU_CLOCK_HZ    12000000
#define TICK_RATE_HZ    1000 // Tang toc SysTick len 1000Hz (1ms moi nhip)
#include <stdint.h>
#include <stddef.h>
#include "kernel.h"
#include "heap.h"
#include "task.h"
// --- Cac bien va dinh nghia Private cua Kernel ---
#define SYSTICK_BASE    0xE000E010
#define STK_CTRL        (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define STK_LOAD        (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define ICSR            (*(volatile uint32_t *)0xE000ED04)
#define PENDSVSET_BIT   (1 << 28)

#define CPU_CLOCK_HZ    12000000
#define TICK_RATE_HZ    1000 // Tang toc SysTick len 1000Hz (1ms moi nhip)

static volatile uint32_t system_ticks = 0;
tcb_t* taskC = 0;   // khai báo thật sự biến taskC

tcb_t* current_task = 0;
tcb_t* task_list_head = 0;

void SysTick_Handler(void) {
    system_ticks++;

    // --- Logic "Danh thuc" moi ---
    if (task_list_head) {
        tcb_t* task = task_list_head;
        do {
            // Neu mot tac vu dang ngu va da den gio
            if (task->state == TASK_SLEEPING && system_ticks >= task->sleep_until_tick) {
                // Danh thuc no day
                task->state = TASK_READY;
            }
            task = task->next;
        } while (task != task_list_head);
    }
    
    // Kich hoat PendSV de bo lap lich co the chay
    ICSR |= PENDSVSET_BIT;
}

//void scheduler_round_robin(void) {
  //  if (current_task) {
    //    if (current_task->state == TASK_RUNNING) current_task->state = TASK_READY;
      //  tcb_t* next_task = current_task;
       // do {
         //   next_task = next_task->next;
          //  if (next_task->state == TASK_READY) {
           //     current_task = next_task;
            //    current_task->state = TASK_RUNNING;
             //   return;
          //  }
      //  } while (next_task != current_task);
    //    if (current_task->state == TASK_READY) current_task->state = TASK_RUNNING;
   // }
//}

void scheduler_priority_based(void)

{
	 if (!task_list_head) return;
	 tcb_t* task = task_list_head;
    tcb_t* next = NULL;
    uint8_t max_priority = 0;


     do {
        if (task->state == TASK_READY && task->priority >= max_priority)
        {
            max_priority = task->priority;
            next = task;
        }
        task = task->next; // ⚠️ PHẢI có dòng này để di chuyển đến task kế tiếp
    } while (task != task_list_head);

    // --- Nếu tìm được task có thể chạy ---
    if (next)
    {
        // Nếu task hiện tại đang RUNNING -> chuyển về READY
        if (current_task && current_task->state == TASK_RUNNING)
            current_task->state = TASK_READY;

        // Cập nhật task mới
        current_task = next;
        current_task->state = TASK_RUNNING;
	// them doan nay de debug
	 uart_puts("[Scheduler] Switching to task: ");
    uart_puti(current_task->task_id);
    uart_puts("\n");
    }
}
// --- API cong cong ---
void kernel_init(void) {
    heap_init();
}

uint32_t kernel_get_ticks(void) {
    return system_ticks;
}

void kernel_add_task(tcb_t* new_task) {
    // Logic them task khong doi
    if (!task_list_head) {
        task_list_head = new_task;
        new_task->next = new_task;
    } else {
        tcb_t* current = task_list_head;
        while (current->next != task_list_head) {
            current = current->next;
        }
        current->next = new_task;
        new_task->next = task_list_head;
    }
}

void __attribute__((naked)) kernel_launch(void) {
    current_task = task_list_head;
    if (current_task) {
        current_task->state = TASK_RUNNING;
    }
    
    STK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1;
    STK_CTRL = (1 << 2) | (1 << 1) | (1 << 0);
    
    __asm("    svc 0");
}
// Ham moi duoc them vao
const tcb_t* kernel_get_task_list_head(void) {
    return task_list_head;
}
// them ham pendsv handler de chuyen doi ngu canh 
//void PendSV_Handler(void) {
    // Gọi bộ lập lịch để chọn task kế tiếp
  //  scheduler_priority_based();
//}
//
__attribute__((naked)) void PendSV_Handler(void)
{  
	// debug
	uart_puts("[PendSV] Context switch\n");
    __asm volatile
    (
        "mrs r0, psp                        \n"
        "ldr r3, =current_task              \n"
        "ldr r2, [r3]                       \n"
        "cbz r2, pendSV_no_save             \n"
        "stmdb r0!, {r4-r11}                \n"
        "str r0, [r2]                       \n"
        "pendSV_no_save:                    \n"
        "bl scheduler_priority_based        \n"
        "ldr r3, =current_task              \n"
        "ldr r1, [r3]                       \n"
        "ldr r0, [r1]                       \n"
        "ldmia r0!, {r4-r11}                \n"
        "msr psp, r0                        \n"
        "bx lr                              \n"
    );
}

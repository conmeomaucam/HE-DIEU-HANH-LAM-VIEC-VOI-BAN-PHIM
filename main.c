// main.c
#include "uart.h"
#include "kernel.h"
#include "task.h"
#include "semaphore.h"


semaphore_t sem_test;          // semaphore dùng để báo có dữ liệu UART
extern volatile int uart_int_count;

static const char* state_to_str(task_state_t st) {
    switch (st) {
        case TASK_READY:   return "READY";
        case TASK_RUNNING: return "RUNNING";
        case TASK_SLEEPING:return "SLEEPING";
        default:           return "UNKNOWN";
    }
}

void taskA_func(void) {
    while (1) {
        uart_puts("[A] running\n");
        
	 semaphore_wait(&sem_test);
	 task_delay(100);

    }
}

void taskB_func(void) {
    while (1) {
        uart_puts("[B] running\n");
	semaphore_signal(&sem_test);
        task_delay(100);
    }
}

void task_monitor_func(void) {
    while (1) {
        task_delay(20);

        uart_puts("\n--- MON ---\n");
        const tcb_t* head = kernel_get_task_list_head();
        if (head) {
            const tcb_t* t = head;
            do {
                uart_puts("ID: ");   uart_puti(t->task_id);
                uart_puts("  Name: ");  uart_puts(t->task_name);
                uart_puts("  State: "); uart_puts(state_to_str(t->state));
                uart_puts("\n");
                t = t->next;
            } while (t != head);
        }

        uart_puts("UART irq cnt = "); uart_puti(uart_int_count); uart_puts("\n");
        uart_puts("------------\n");
    }
}

void taskC_func(void) {
    while (1) {
        // dùng semaphore nếu ISR đã signal; hoặc có thể check uart_available()
        semaphore_wait(&sem_test);
        char c = uart_getchar();
        uart_puts("[C] key: ");
        uart_putchar(c);
        uart_puts("\n");
        task_delay(100);   // nhường CPU 1 tick để A/B xen kẽ mượt
    }
}

int main(void) {
    kernel_init();
    uart_init();

    // Semaphore bắt đầu = 0 để “chờ sự kiện”
    semaphore_init(&sem_test,0);

    uart_puts("Booting...\n");

    tcb_t* taskA    = task_create(taskA_func, "Task A", 10, 512);
    tcb_t* taskB    = task_create(taskB_func, "Task B", 10, 512);
    tcb_t* monitor  = task_create(task_monitor_func, "Monitor", 20, 512);
    tcb_t* taskC    = task_create(taskC_func, "Task C", 5,  512);

    kernel_add_task(taskA);
    kernel_add_task(taskB);
    kernel_add_task(monitor);
    kernel_add_task(taskC);

    uart_puts("Tasks ready. Launching kernel...\n");
    kernel_launch();

    while (1) { /* never here */ }
}


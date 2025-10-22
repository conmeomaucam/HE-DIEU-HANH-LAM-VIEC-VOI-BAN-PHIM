// main.c
#include "uart.h"
#include "kernel.h"
#include "task.h"

extern tcb_t* taskC;

static const char* state_to_str(task_state_t st) {
    switch (st) {
        case TASK_READY: return "READY";
        case TASK_RUNNING: return "RUNNING";
        case TASK_SLEEPING: return "SLEEPING";
        default: return "UNKNOWN";
    }
}

void taskA_func(void) {
    while (1) {
        uart_puts("[A] running\n");
        task_delay(500);
    }
}
void taskB_func(void) {
    while (1) {
        uart_puts("[B] running\n");
        task_delay(500);
    }
}
void task_monitor_func(void) {
    while (1) {
        task_delay(1000);
        uart_puts("\n--- MON ---\n");
        const tcb_t* head = kernel_get_task_list_head();
        if (head) {
            const tcb_t* t = head;
            do {
                uart_puts("ID: "); uart_puti(t->task_id);
                uart_puts("  Name: "); uart_puts(t->task_name);
                uart_puts("  State: "); uart_puts(state_to_str(t->state));
                uart_puts("\n");
                t = t->next;
            } while (t != head);
        }
        extern volatile int uart_int_count;
        uart_puts("UART irq cnt = "); uart_puti(uart_int_count); uart_puts("\n");
        uart_puts("------------\n");
    }
}

void taskC_func(void) {
    while (1) {
        if (uart_available()) {
            char c = uart_getchar();
            uart_puts("[C] key: ");
            uart_putchar(c);
            uart_puts("\n");
            for (volatile int i = 0; i < 300000; ++i) { /* giả lập xử lý */ }
        }
        task_delay(100);
    }
}

int main(void) {
    kernel_init();
    uart_init();

    uart_puts("Booting...\n");

    tcb_t* taskA   = task_create(taskA_func, "Task A", 10, 512);
    tcb_t* taskB   = task_create(taskB_func, "Task B", 10, 512);
    tcb_t* monitor = task_create(task_monitor_func, "Monitor", 20, 512);
    tcb_t* TaskC   = task_create(taskC_func, "Task C",  5,  512);

    kernel_add_task(taskA);
    kernel_add_task(taskB);
    kernel_add_task(monitor);
    kernel_add_task(TaskC);

    taskC = TaskC;   // <<< QUAN TRỌNG: cho UART ISR đánh thức Task C

    uart_puts("Tasks ready. Launching kernel...\n");
    kernel_launch();

    while (1) { /* never here */ }
}


#include "uart.h"
#include "kernel.h"
#include "task.h"

// Ham chuyen doi trang thai enum sang chuoi de in
const char* state_to_str(task_state_t state) {
    switch(state) {
        case TASK_READY: return "READY";
        case TASK_RUNNING: return "RUNNING";
        case TASK_SLEEPING: return "SLEEPING";
        default: return "UNKNOWN";
    }
}

// Tac vu A: In 'A' moi 500ms
void taskA_func(void) {
    while(1) {
        uart_puts(" TASK ID 1 DANG LAM VIEC  ");
        task_delay(500);
    }
}

// Tac vu B: In 'B' moi 500ms
void taskB_func(void) {
    while(1) {
        uart_puts("TASK B ID 2  ");
        task_delay(500);
    }
}

// Tac vu C (Giam sat): In trang thai cua tat ca cac tac vu moi 5 giay
void task_monitor_func(void) {
    while(1) {
        // THAY DOI QUAN TRONG: Ngu 5 giay
        task_delay(5000);

        uart_puts("\n--- TASK MONITOR (");
        uart_puti(kernel_get_ticks());
        uart_puts(" ms) ---\n");
        const tcb_t* head = kernel_get_task_list_head();
        if (head) {
            const tcb_t* task = head;
            do {
                uart_puts("ID: "); uart_puti(task->task_id);
                uart_puts(" | Name: "); uart_puts(task->task_name);
                uart_puts(" | State: "); uart_puts(state_to_str(task->state));
                uart_puts("\n");
                task = task->next;
            } while (task != head);
        }
        uart_puts("--------------------------\n");
    }
}
// task ban phim 
void taskC_func(void) {
    while (1) {
	    uart_puts("[DEBUG] Task C da duoc goi!\n");
        if (uart_available()) {
            char c = uart_getchar();
            uart_puts("[TaskC] Xu ly lenh tu ban phim...\n");
            for (volatile int i = 0; i < 500000; i++); // giả lập xử lý nặng
            uart_puts("[TaskC] Hoan thanh xu ly!\n");
        }
        task_delay(100);
    }
}


int main(void) {
    kernel_init();
    uart_init();
    uart_puts("EonOS Dang Khoi Dong... Giai Doan 11: Giam Sat He Thong\n");
    
    tcb_t* taskA = task_create(taskA_func, "Task A", 10, 512);
    tcb_t* taskB = task_create(taskB_func, "Task B", 10, 512);
    tcb_t* monitor = task_create(task_monitor_func, "Monitor", 20, 512);
    tcb_t* TaskC = task_create(taskC_func, "Task C" , 0 , 512);
    kernel_add_task(taskA);
    kernel_add_task(taskB);
    kernel_add_task(monitor);
    kernel_add_task(TaskC);
    
    uart_puts("Da them 4 tac vu. Bat dau bo lap lich...\n");
    
    kernel_launch();
//   uart_init();
//    uart_puts("\n=== UART ECHO MODE ===\nGo phim bat ky, toi se echo lai:\n");

  //  while (1) {
    //    char c = uart_getchar();  // Chờ và đọc phím
      //  uart_putchar(c);          // In lại ký tự  
   // return 0;
//}
}

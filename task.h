#ifndef TASK_H
#define TASK_H

#include <stdint.h>

/*
 * task.h: Dinh nghia cac cau truc du lieu cot loi cho RTOS.
 */

// Dinh nghia cac trang thai co the co cua mot tac vu
typedef enum {
    TASK_READY,     // San sang de chay
    TASK_RUNNING,   // Dang duoc CPU thuc thi
    TASK_SLEEPING,  // Dang "ngu", cho den mot thoi diem nhat dinh
     TASK_BLOCKED ,
} task_state_t;

// "Ho so nhan su" chi tiet cua moi tac vu
typedef struct TCB {
    void* stack_ptr;          // Con tro ngan xep hien tai
    struct TCB* next;           // Con tro den TCB tiep theo trong danh sach

    // --- Cac truong moi ---
    uint32_t task_id;           // ID duy nhat cua tac vu
    const char* task_name;      // Ten cua tac vu (de go loi)
    task_state_t state;         // Trang thai hien tai cua tac vu
    uint8_t priority;           // Do uu tien (so cang nho, uu tien cang cao)
    uint32_t sleep_until_tick;  // Thoi diem bao thuc

    // Thong tin de kiem tra tran ngan xep
    void* stack_base;         // Dia chi goc cua vung nho ngan xep
    uint32_t stack_size;        // Kich thuoc cua ngan xep

} tcb_t;


// Cap nhat prototype de nhan them tham so
tcb_t* task_create(
    void (*task_function)(void),
    const char* name,
    uint8_t priority,
    uint32_t stack_size
);

void task_delay(uint32_t ms);


#endif // TASK_H

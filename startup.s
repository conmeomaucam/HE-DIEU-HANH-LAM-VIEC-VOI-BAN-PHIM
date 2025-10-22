/* startup.s */
.syntax unified
.cpu cortex-m3
.thumb

.extern main
.extern current_task
.extern scheduler_priority_based
.extern UART0_Handler
/* SVC_Handler dùng để khởi động task đầu tiên (đã viết dưới) */

.weak NMI_Handler, HardFault_Handler, SVC_Handler, PendSV_Handler, SysTick_Handler
.thumb_set NMI_Handler, Default_Handler
.thumb_set HardFault_Handler, Default_Handler

/* Bảng vector: core exceptions + external IRQs (UART0 = IRQ5) */
.section .vectors, "a", %progbits
.word _estack
.word _reset_handler + 1
.word NMI_Handler + 1
.word HardFault_Handler + 1
.word 0            /* MemManage (M3 có nhưng tắt) */
.word 0            /* BusFault */
.word 0            /* UsageFault */
.word 0, 0, 0, 0   /* Reserved */
.word SVC_Handler + 1
.word 0            /* DebugMon */
.word 0            /* Reserved */
.word PendSV_Handler + 1
.word SysTick_Handler + 1

/* External IRQs (bắt đầu từ IRQ0) */
.word 0   /* GPIO Port A */
.word 0   /* GPIO Port B */
.word 0   /* GPIO Port C */
.word 0   /* GPIO Port D */
.word 0   /* GPIO Port E */
.word UART0_Handler + 1    /* IRQ5: UART0 */
.word 0   /* IRQ6: UART1 */
/* ... nếu cần thêm IRQ khác, điền tiếp theo đúng thứ tự ... */

.section .text
.global _reset_handler
_reset_handler:
    /* copy .data */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:  cmp r1, r2
    bhs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b
2:
    /* zero .bss */
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
3:  cmp r1, r2
    bhs 4f
    str r3, [r1], #4
    b 3b
4:
    bl main
Default_Handler:
    b Default_Handler

/* SVC: nạp PSP từ current_task->stack_ptr, chuyển sang Thread+PSP */
.global SVC_Handler
.thumb_func
SVC_Handler:
    ldr r0, =current_task    /* r0 = &current_task */
    ldr r1, [r0]             /* r1 = current_task */
    ldr r0, [r1]             /* r0 = current_task->stack_ptr */
    ldmia r0!, {r4-r11}      /* khôi phục r4..r11 (SW-saved) */
    msr psp, r0              /* PSP = stack sau khi pop r4..r11 */
    movs r0, #2
    msr control, r0          /* Thread mode, use PSP */
    isb
    ldr r0, =0xFFFFFFFD      /* EXC_RETURN: back to Thread+PSP */
    bx  r0

/* PendSV: save r4-r11 của task cũ, gọi scheduler chọn task mới, restore r4-r11 */
.global PendSV_Handler
.thumb_func
PendSV_Handler:
    mrs r0, psp
    cbz r0, pend_no_save
    stmdb r0!, {r4-r11}          /* save SW context */
    ldr r1, =current_task
    ldr r2, [r1]
    str r0, [r2]                 /* current_task->stack_ptr = PSP */
pend_no_save:
    bl scheduler_priority_based   /* chọn current_task mới (đặt state RUNNING) */
    ldr r1, =current_task
    ldr r2, [r1]
    ldr r0, [r2]                 /* PSP = new->stack_ptr */
    ldmia r0!, {r4-r11}          /* restore SW context */
    msr psp, r0
    ldr lr, =0xFFFFFFFD          /* return to Thread+PSP */
    bx  lr

/* SysTick do bạn định nghĩa bên C (kernel.c) */


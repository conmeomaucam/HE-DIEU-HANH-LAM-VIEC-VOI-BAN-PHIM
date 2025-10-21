.syntax unified
.cpu cortex-m3
.thumb
.extern scheduler_priority_based
.extern PendSV_Handler

.extern current_task
.weak NMI_Handler, HardFault_Handler, SVC_Handler, PendSV_Handler, SysTick_Handler
.thumb_set NMI_Handler, Default_Handler
.thumb_set HardFault_Handler, Default_Handler
.section .vectors
.word _estack, _reset_handler + 1, NMI_Handler + 1, HardFault_Handler + 1
.word 0, 0, 0, 0, 0, 0, 0, SVC_Handler + 1, 0, 0, PendSV_Handler + 1, SysTick_Handler + 1
.word UART0_Handler + 1 // IRQ 5 (UART0)
.word PendSV_Handler + 1

.word 0                 // IRQ 6 (UART1)
.section .text
.global _reset_handler
_reset_handler:
    ldr r0,=_sidata; ldr r1,=_sdata; ldr r2,=_edata
copy_loop: cmp r1,r2; bhs copy_end; ldr r3,[r0],#4; str r3,[r1],#4; b copy_loop
copy_end:
    ldr r1,=_sbss; ldr r2,=_ebss; movs r3,#0
zero_loop: cmp r1,r2; bhs zero_end; str r3,[r1],#4; b zero_loop
zero_end:
    bl main
Default_Handler:
hang: b hang

.global SVC_Handler
.thumb_func
SVC_Handler:
    ldr r0, =current_task
    ldr r1, [r0]
    ldr r0, [r1]
    ldmia r0!, {r4-r11}
    msr psp, r0
    mov r0, #2
    msr control, r0
    isb
    ldr r0, =0xFFFFFFFD
    bx r0

.global PendSV_Handler
.thumb_func
PendSV_Handler:
    mrs r0, psp
    stmdb r0!, {r4-r11}
    ldr r1, =current_task
    ldr r2, [r1]
    str r0, [r2]
    bl scheduler_priority_based 
    ldr r2, [r1]
    ldr r0, [r2]
    ldmia r0!, {r4-r11}
    msr psp, r0
    ldr lr , = 0xFFFFFFFD @
    bx lr

// uart.c
#include <stdint.h>
#include "task.h"
#include "uart.h"
#include "kernel.h"

#define UART0_BASE      0x4000C000
#define UART_DR         (*(volatile uint32_t *)(UART0_BASE + 0x000))
#define UART_FR         (*(volatile uint32_t *)(UART0_BASE + 0x018))
#define UART_IM         (*(volatile uint32_t *)(UART0_BASE + 0x038))
#define UART_ICR        (*(volatile uint32_t *)(UART0_BASE + 0x044))
#define UART_FR_RXFE    (1u << 4)
#define UART_IM_RXIM    (1u << 4)

#define NVIC_BASE       0xE000E100
#define NVIC_EN0        (*(volatile uint32_t *)(NVIC_BASE + 0x00))

#define ICSR            (*(volatile uint32_t *)0xE000ED04)


#define RX_BUFFER_SIZE  256
static volatile char rx_buffer[RX_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

extern tcb_t* taskC;

volatile int uart_int_count = 0;  // debug

void UART0_Handler(void) {
    char c = (char)UART_DR;                    // đọc dữ liệu
    uint16_t nh = (rx_head + 1) % RX_BUFFER_SIZE;
    if (nh != rx_tail) {                       // tránh tràn vòng
        rx_buffer[rx_head] = c;
        rx_head = nh;
    }
    uart_int_count++;                          // debug: đếm ngắt

    if (taskC) {                               // đánh thức Task C nếu đã gán
        taskC->state = TASK_READY;
        ICSR |= PENDSVSET_BIT;
    }

    // Tùy QEMU/SoC có cần clear cờ hay không
    // UART_ICR = (1u << 4);
}

void uart_init(void) {
    // Bật RX interrupt + NVIC UART0 (IRQ5)
    UART_IM |= UART_IM_RXIM;
    NVIC_EN0 |= (1u << 5);
}
 void uart_putchar(char c) {
    while (UART_FR & (1u<<5)) { /* TXFF full -> chờ */ }
    UART_DR = c;
}

void uart_puts(const char* s) {
    while (*s) uart_putchar(*s++);
}

int uart_available(void) {
    return rx_head != rx_tail;
}

char uart_getchar(void) {
    while (rx_head == rx_tail) { /* block */ }
    char c = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
    return c;
}

void uart_puti(int v) {
    char buf[12];
    int i = 0, neg = 0;
    if (v == 0) { uart_putchar('0'); return; }
    if (v < 0) { neg = 1; v = -v; }
    while (v) { buf[i++] = '0' + (v % 10); v /= 10; }
    if (neg) buf[i++] = '-';
    while (i--) uart_putchar(buf[i]);
}


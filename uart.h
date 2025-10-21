#ifndef UART_H
#define UART_H
#define UART_CR   (*(volatile uint32_t *)(UART0_BASE + 0x030))
#define UART_CR_TXE (1 << 8)
#define UART_CR_RXE (1 << 9)
#define UART_CR_UARTEN (1 << 0)
void uart_init(void);
void uart_putchar(char c);
void uart_puts(const char* s);
void uart_puti(int value);
char uart_getchar(void);
#endif

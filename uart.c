#include <stdint.h>
#include "uart.h"

#define UART0_BASE 0x4000C000
#define UART_DR    (*(volatile uint32_t *)(UART0_BASE + 0x000))
#define UART_FR    (*(volatile uint32_t *)(UART0_BASE + 0x018))
#define UART_FR_TXFF (1 << 5)


#define UART_IM         (*(volatile uint32_t *)(UART0_BASE + 0x038)) // Thanh ghi Mat na Ngat
#define UART_ICR        (*(volatile uint32_t *)(UART0_BASE + 0x044)) // Thanh ghi Xoa Ngat

#define UART_FR_RXFE    (1 << 4)  // Co bao bo dem nhan RONG
#define UART_IM_RXIM    (1 << 4)  // Bit bat/tat ngat khi nhan du lieu

// Dinh nghia cho bo dieu khien ngat trung tam (NVIC)
#define NVIC_BASE       0xE000E100
#define NVIC_EN0        (*(volatile uint32_t *)(NVIC_BASE + 0x00)) // Thanh ghi bat ngat
								   //
								   //
#define RX_BUFFER_SIZE 256
static volatile  char rx_buffer[RX_BUFFER_SIZE];
// 'volatile' de bao cho trinh bien dich khong toi uu hoa cac bien nay,
// vi chung co the duoc thay doi boi mot trinh xu ly ngat bat cu luc nao.
static volatile uint16_t rx_buffer_head = 0; // Vi tri de ghi ky tu moi vao
static volatile uint16_t rx_buffer_tail = 0; // Vi tri de doc ky tu ra

/**
 * @brief "Nguoi nhan thu" - Trinh xu ly ngat cho UART0.
 * Ham nay duoc phan cung goi tu dong.
 */
void UART0_Handler(void) {
    // Doc ky tu tu thanh ghi du lieu
    char c = (char)UART_DR;

    // Tinh vi tri tiep theo cho con tro head
    uint16_t next_head = (rx_buffer_head + 1) % RX_BUFFER_SIZE;

    // Chi them vao buffer neu no chua day (de tranh ghi de len du lieu cu)
    if (next_head != rx_buffer_tail) {
        rx_buffer[rx_buffer_head] = c;
        rx_buffer_head = next_head;
    }
    
    // (Khong can xoa co ngat tren QEMU, nhung tren phan cung that se can)
    // UART_ICR = (1 << 4);
}

void uart_init(void) {
 	// 1. Bat ngat nhan (Receive Interrupt Mask) trong module UART
    UART_IM |= UART_IM_RXIM;

    // 2. Bat duong day ngat so 5 (cua UART0) trong bo dieu khien NVIC
    NVIC_EN0 |= (1 << 5);

}
void uart_putchar(char c) {
       	while (UART_FR & UART_FR_TXFF);
       	UART_DR = c;
}
void uart_puts(const char* s) {
       	while (*s) { uart_putchar(*s++);
       	}
}
char uart_getchar(void) {
    // Cho cho den khi co du lieu trong bo dem (head != tail)
    while (rx_buffer_head == rx_buffer_tail);

    // Lay ky tu ra khoi bo dem
    char c = rx_buffer[rx_buffer_tail];
    // Dich chuyen con tro tail den vi tri tiep theo
    rx_buffer_tail = (rx_buffer_tail + 1) % RX_BUFFER_SIZE;

    return c;
}

// In 1 ký tự
extern void uart_putchar(char c);

// Hàm in số nguyên (32-bit) ra UART
void uart_puti(int value) {
    char buffer[12];  // đủ chứa số âm lớn nhất: -2147483648
    int i = 0;
    int is_negative = 0;

    if (value == 0) {
        uart_putchar('0');
        return;
    }

    // Nếu số âm
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    // Chuyển từng chữ số vào buffer (theo thứ tự ngược)
    while (value > 0) {
        buffer[i++] = (value % 10) + '0';  // chuyển số cuối sang ký tự
        value /= 10;
    }

    // Nếu là số âm, thêm dấu '-'
    if (is_negative) {
        buffer[i++] = '-';
    }

    // In ngược lại buffer
    while (--i >= 0) {
        uart_putchar(buffer[i]);
    }
}



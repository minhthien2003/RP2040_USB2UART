
/**
 * uart_pio.h File
 * Uart init using PIO component of RP2040
 * Author: Nguyen Minh Thien
 * Ver: 1.0
 *Year: 1,2026
 */



#include "stdint.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "uart_rx_tx.pio.h"





void uart_tx_program_init(PIO pio, unsigned int sm, unsigned int offset, unsigned int pin_tx, unsigned int baud);
void uart_tx_program_putc(PIO pio, unsigned int sm, char c);
void uart_tx_program_puts(PIO pio, unsigned int sm, const char *s);
void uart_rx_program_init(PIO pio, unsigned int sm, unsigned int offset, unsigned int pin, unsigned int baud);
char uart_rx_program_getc(PIO pio, unsigned int sm);
void pio_uart_set_baudrate(PIO pio, uint sm_tx, uint sm_rx, uint32_t baud);
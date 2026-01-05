#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"
#include "hardware/pio.h"
#include "uart_rx_tx.pio.h"







// ===== UART CONFIG =====
#define UART0_ID uart0
#define UART1_ID uart1

#define UART0_TX 0
#define UART0_RX 1

#define UART1_TX 4
#define UART1_RX 5

#define PIO_UART2_TX_PIN    2
#define PIO_UART2_RX_PIN    3


#define UART_BAUD_DEFAULT 115200




PIO pio;
uint smTx = 0;
uint smRx = 1;
uint tx_offset;
uint rx_offset;




// =======================

void uart_init_all(void) {
    // UART0
    uart_init(UART0_ID, UART_BAUD_DEFAULT);
    gpio_set_function(UART0_TX, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX, GPIO_FUNC_UART);

    // UART1
    uart_init(UART1_ID, UART_BAUD_DEFAULT);
    gpio_set_function(UART1_TX, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX, GPIO_FUNC_UART);
}

// USB CDC -> UART
void usb_to_uart(void) {
    uint8_t buf[64];

#if 0
    if (tud_cdc_n_available(0)) {
        uint32_t n = tud_cdc_n_read(0, buf, sizeof(buf));
        
        for (uint8_t i = 0; i < n; i++)
        {
           uart_tx_program_putc(pio, smTx, buf[i]);
        }
            
    }

#endif

    // CDC0 -> UART0
    if (tud_cdc_n_available(0)) {
        uint32_t n = tud_cdc_n_read(0, buf, sizeof(buf));
        uart_write_blocking(UART0_ID, buf, n);
    }

    // CDC1 -> UART1
    if (tud_cdc_n_available(1)) {
        uint32_t n = tud_cdc_n_read(1, buf, sizeof(buf));
        uart_write_blocking(UART1_ID, buf, n);
    }
#

}

// UART -> USB CDC
void uart_to_usb(void) {

#if 0
    char c = uart_rx_program_getc(pio, smRx);
    tud_cdc_n_write_char(0, c);
#endif
    // UART0 -> CDC0
    while (uart_is_readable(UART0_ID)) {
        uint8_t ch = uart_getc(UART0_ID);
        tud_cdc_n_write_char(0, ch);
    }
    tud_cdc_n_write_flush(0);

    // UART1 -> CDC1
    while (uart_is_readable(UART1_ID)) {
        uint8_t ch = uart_getc(UART1_ID);
        tud_cdc_n_write_char(1, ch);
    }


    tud_cdc_n_write_flush(0);

}

// Map baudrate từ PC xuống UART
void tud_cdc_line_coding_cb(uint8_t itf,
                            cdc_line_coding_t const* p) {
    if (itf == 0) {
        uart_set_baudrate(UART0_ID, p->bit_rate);
    } else if (itf == 1) {
        uart_set_baudrate(UART1_ID, p->bit_rate);
    }
}


void PIO_UARTInit(void)
{
    bool successTx = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_tx_program, &pio, &smTx, &tx_offset, PIO_UART2_TX_PIN, 1, true);
    hard_assert(successTx);
    bool successRx = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_rx_program, &pio, &smRx, &rx_offset, PIO_UART2_RX_PIN, 1, true);
    hard_assert(successRx);

    uart_tx_program_init(pio, smTx, tx_offset, PIO_UART2_TX_PIN, UART_BAUD_DEFAULT);
    uart_rx_program_init(pio, smRx, rx_offset, PIO_UART2_RX_PIN, UART_BAUD_DEFAULT);
}

int main(void) {
    stdio_init_all();
    uart_init_all();

    tud_init(0);
    PIO_UARTInit();

    while (true) {
        tud_task();      // USB stack
        usb_to_uart();   // USB -> UART
        uart_to_usb();   // UART -> USB
    }
}

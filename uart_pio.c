

/**
 * uart_pio.c File
 * Uart init using PIO component of RP2040
 * Author: Nguyen Minh Thien
 * Ver: 1.0
 *Year: 1,2026
 */
#include "uart_pio.h"





void uart_tx_program_init(PIO pio, unsigned int sm, unsigned int offset, unsigned int pin_tx, unsigned int baud) {
    // Tell PIO to initially drive output-high on the selected pin, then map PIO
    // onto that pin with the IO muxes.
    pio_sm_set_pins_with_mask64(pio, sm, 1ull << pin_tx, 1ull << pin_tx);
    pio_sm_set_pindirs_with_mask64(pio, sm, 1ull << pin_tx, 1ull << pin_tx);
    pio_gpio_init(pio, pin_tx);

    pio_sm_config c = uart_tx_program_get_default_config(offset);

    // OUT shifts to right, no autopull
    sm_config_set_out_shift(&c, true, false, 32);

    // We are mapping both OUT and side-set to the same pin, because sometimes
    // we need to assert user data onto the pin (with OUT) and sometimes
    // assert constant values (start/stop bit)
    sm_config_set_out_pins(&c, pin_tx, 1);
    sm_config_set_sideset_pins(&c, pin_tx);

    // We only need TX, so get an 8-deep FIFO!
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    // SM transmits 1 bit per 8 execution cycles.
    float div = (float)clock_get_hz(clk_sys) / (8 * baud);
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}




void uart_tx_program_putc(PIO pio, unsigned int sm, char c) {
    pio_sm_put_blocking(pio, sm, (uint32_t)c);
}

void uart_tx_program_puts(PIO pio, unsigned int sm, const char *s) {
    while (*s)
        uart_tx_program_putc(pio, sm, *s++);
}






void uart_rx_program_init(PIO pio, unsigned int sm, unsigned int offset, unsigned int pin, unsigned int baud) {
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, false);
    pio_gpio_init(pio, pin);
    gpio_pull_up(pin);

    pio_sm_config c = uart_rx_program_get_default_config(offset);
    sm_config_set_in_pins(&c, pin); // for WAIT, IN
    sm_config_set_jmp_pin(&c, pin); // for JMP
    // Shift to right, autopush disabled
    sm_config_set_in_shift(&c, true, false, 32);
    // Deeper FIFO as we're not doing any TX
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    // SM transmits 1 bit per 8 execution cycles.
    float div = (float)clock_get_hz(clk_sys) / (8 * baud);
    sm_config_set_clkdiv(&c, div);
    
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}





char uart_rx_program_getc(PIO pio, unsigned int sm) {
    // 8-bit read from the uppermost byte of the FIFO, as data is left-justified
    io_rw_8 *rxfifo_shift = (io_rw_8*)&pio->rxf[sm] + 3;
    while (pio_sm_is_rx_fifo_empty(pio, sm))
        tight_loop_contents();
    return (char)*rxfifo_shift;
}


void pio_uart_set_baudrate(PIO pio, uint sm_tx, uint sm_rx, uint32_t baud)
{
    float div = (float)clock_get_hz(clk_sys) / (8.0f * baud);

    pio_sm_set_enabled(pio, sm_tx, false);
    pio_sm_set_enabled(pio, sm_rx, false);

    pio_sm_set_clkdiv(pio, sm_tx, div);
    pio_sm_set_clkdiv(pio, sm_rx, div);

    pio_sm_clear_fifos(pio, sm_tx);
    pio_sm_clear_fifos(pio, sm_rx);

    pio_sm_set_enabled(pio, sm_tx, true);
    pio_sm_set_enabled(pio, sm_rx, true);
}


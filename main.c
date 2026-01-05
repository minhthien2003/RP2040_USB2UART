
/**
 * main.c File
 * Author: Nguyen Minh Thien
 * ver 1.0
 * Time: 1,2026
 */


#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"
#include "uart_pio.h"




// ===== UART CONFIG =====
#define UART0_ID uart0
#define UART1_ID uart1

#define UART0_TX 0
#define UART0_RX 1

#define UART1_TX 4
#define UART1_RX 5

//PIO UART2
#define PIO_UART2_TX_PIN    2
#define PIO_UART2_RX_PIN    3
//PIO UART3
#define PIO_UART3_TX_PIN    6
#define PIO_UART3_RX_PIN    7

#define UART_BAUD_DEFAULT 115200
#define USB_DATA_BUFF_SIZE  64

#define MAX_PIO_UART_NUM    2

enum
{
    PIO_UART_0 = 0,
    PIO_UART_1,
};




PIO pio;


typedef struct
{
    uint u32TX_PIN;
    uint u32RX_PIN;
    uint u32SMTX;
    uint u32SMRX;
    uint u32TXOffset;
    uint u32RXOffset;
    bool successTx;
    bool successRx;
    uint u32UART_BAUD_DEFAULT;

} PIO_UART_t;



PIO_UART_t gPIO_UART_Table[MAX_PIO_UART_NUM] = 
{
    {
    .u32RX_PIN      = PIO_UART2_RX_PIN,
    .u32TX_PIN      = PIO_UART2_TX_PIN,
    .u32SMTX        = 0,
    .u32SMRX        = 0,
    .u32TXOffset    = 0,
    .u32RXOffset    = 0,
    .successTx      = 0,
    .successRx      = 0,
    .u32UART_BAUD_DEFAULT = UART_BAUD_DEFAULT,
},

{

    .u32RX_PIN      = PIO_UART3_RX_PIN,
    .u32TX_PIN      = PIO_UART3_TX_PIN,
    .u32SMTX        = 0,
    .u32SMRX        = 0,
    .u32TXOffset    = 0,
    .u32RXOffset    = 0,
    .successTx      = 0,
    .successRx      = 0,
    .u32UART_BAUD_DEFAULT = UART_BAUD_DEFAULT,

}

};


uint8_t USB_DATA_BUFFER[USB_DATA_BUFF_SIZE];


// =======================
static inline void pio_uart_write_bytes(uint8_t cdc_itf, PIO pio, uint sm, uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;

    while (i < len)
    {
        // if TX FIFO not full
        if (!pio_sm_is_tx_fifo_full(pio, sm))
        {
            pio_sm_put(pio, sm, buf[i]);
            i++;
        }
        else
        {
            // Handle for USB stack
            tud_task();
        }
    }
}


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
    

    // CDC0 -> UART0
    if (tud_cdc_n_available(0)) {
        uint32_t n0 = tud_cdc_n_read(0, USB_DATA_BUFFER, USB_DATA_BUFF_SIZE);
        uart_write_blocking(UART0_ID, USB_DATA_BUFFER, n0);
    }

    // CDC1 -> UART1
    if (tud_cdc_n_available(1)) {
        uint32_t n1 = tud_cdc_n_read(1, USB_DATA_BUFFER, USB_DATA_BUFF_SIZE);
        uart_write_blocking(UART1_ID, USB_DATA_BUFFER, n1);
    }
    //CDC2 - > UART2
    if (tud_cdc_n_available(2))
    {
        uint32_t n2 = tud_cdc_n_read(2, USB_DATA_BUFFER, USB_DATA_BUFF_SIZE);
        pio_uart_write_bytes(2, pio, gPIO_UART_Table[0].u32SMTX, USB_DATA_BUFFER, n2);
    }

    //CDC3 - > UART3
    if (tud_cdc_n_available(3))
    {
        uint32_t n3 = tud_cdc_n_read(3, USB_DATA_BUFFER, USB_DATA_BUFF_SIZE);
        pio_uart_write_bytes(3, pio, gPIO_UART_Table[1].u32SMTX, USB_DATA_BUFFER, n3);
    }
    

}

// UART -> USB CDC
void uart_to_usb(void) 
{

     // UART0 -> CDC0
    while (uart_is_readable(UART0_ID)) {
        tud_cdc_n_write_char(0, uart_getc(UART0_ID));
    }
    tud_cdc_n_write_flush(0);

    // UART1 -> CDC1
    while (uart_is_readable(UART1_ID)) {
        tud_cdc_n_write_char(1, uart_getc(UART1_ID));
    }
    tud_cdc_n_write_flush(1);

    // PIO UART2 RX
    while (!pio_sm_is_rx_fifo_empty(pio, gPIO_UART_Table[0].u32SMRX)) {
        char c = uart_rx_program_getc(pio, gPIO_UART_Table[0].u32SMRX);
        tud_cdc_n_write_char(2, c);
    }
    tud_cdc_n_write_flush(2);

    // PIO UART3 RX
    while (!pio_sm_is_rx_fifo_empty(pio, gPIO_UART_Table[1].u32SMRX)) {
        char c = uart_rx_program_getc(pio, gPIO_UART_Table[1].u32SMRX);
        tud_cdc_n_write_char(3, c);
    }
    tud_cdc_n_write_flush(3);

}

// Set baudrate from PC to MCU
void tud_cdc_line_coding_cb(uint8_t itf,   cdc_line_coding_t const* p) 
{

    switch (itf)
    {
    case 0:
        uart_set_baudrate(UART0_ID, p->bit_rate);
        break;
    case 1:
        uart_set_baudrate(UART1_ID, p->bit_rate);
        break;
    case 2:
        pio_uart_set_baudrate(pio, gPIO_UART_Table[0].u32SMTX, gPIO_UART_Table[0].u32SMRX, p->bit_rate);
        break;
    case 3:
        pio_uart_set_baudrate(pio, gPIO_UART_Table[1].u32SMTX, gPIO_UART_Table[1].u32SMRX, p->bit_rate); //Note, if use pio2, please modify in here
        break;

    default:
        break;
    }
    
}


void PIO_UARTInit(PIO_UART_t * pPIOUART)
{
    //Handler for Tx

    pPIOUART->successTx = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_tx_program, \
        &pio, &(pPIOUART->u32SMTX), &(pPIOUART->u32TXOffset), pPIOUART->u32TX_PIN ,1, true );

    hard_assert(pPIOUART->successTx);

    //Handle for Rx
    pPIOUART->successRx = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_rx_program, \ 
        &pio, &(pPIOUART->u32SMRX), &(pPIOUART->u32RXOffset), pPIOUART->u32RX_PIN, 1, true);
    
    hard_assert(pPIOUART->successRx);
    


    //Call pio program Init
    uart_tx_program_init(pio, pPIOUART->u32SMTX, pPIOUART->u32TXOffset, pPIOUART->u32TX_PIN, pPIOUART->u32UART_BAUD_DEFAULT);
    uart_rx_program_init(pio, pPIOUART->u32SMRX, pPIOUART->u32RXOffset, pPIOUART->u32RX_PIN, pPIOUART->u32UART_BAUD_DEFAULT);

}

int main(void) {
    stdio_init_all();
    uart_init_all();

    //USB platform init
    tud_init(0);

    //PIO Unit init
    for (uint8_t i = 0; i < MAX_PIO_UART_NUM; i++)
    {
       PIO_UARTInit(&gPIO_UART_Table[i]);
    }
    

    //Entry of loop
    while (true) {
        tud_task();      // USB stack
        usb_to_uart();   // USB -> UART
        uart_to_usb();   // UART -> USB
    }
}

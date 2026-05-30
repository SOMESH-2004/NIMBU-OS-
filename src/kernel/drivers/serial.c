/**
 * @file serial.c
 * @brief Serial port (UART) driver implementation
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Implements COM1 serial port driver at 115200 baud.
 * Used for debug output via serial console.
 */

#include "driver.h"

#define UART_DATA       (COM1_PORT + 0)
#define UART_IER        (COM1_PORT + 1)
#define UART_FCR        (COM1_PORT + 2)
#define UART_LCR        (COM1_PORT + 3)
#define UART_MCR        (COM1_PORT + 4)
#define UART_LSR        (COM1_PORT + 5)
#define UART_MSR        (COM1_PORT + 6)
#define UART_SR         (COM1_PORT + 7)

/* Line Status Register bits */
#define LSR_DR          0x01  /* Data ready */
#define LSR_THRE        0x20  /* TX holding register empty */

/**
 * @brief Initialize serial port
 */
int serial_init(void) {
    uint8_t byte;
    
    /* Disable all interrupts */
    byte = 0x00;
    __asm__("out %0, %1" : : "a"(byte), "d"(UART_IER));
    
    /* Enable DLAB to set baud rate */
    byte = 0x80;
    __asm__("out %0, %1" : : "a"(byte), "d"(UART_LCR));
    
    /* Set baud rate to 115200 (divisor = 1) */
    byte = 0x01;
    __asm__("out %0, %1" : : "a"(byte), "d"(UART_DATA));
    __asm__("out %0, %1" : : "a"(byte), "d"(UART_IER));
    
    /* Disable DLAB, set 8N1 */
    byte = 0x03;
    __asm__("out %0, %1" : : "a"(byte), "d"(UART_LCR));
    
    /* Enable FIFO, clear TX/RX FIFO */
    byte = 0xC7;
    __asm__("out %0, %1" : : "a"(byte), "d"(UART_FCR));
    
    /* Enable interrupts */
    byte = 0x0B;
    __asm__("out %0, %1" : : "a"(byte), "d"(UART_MCR));
    
    /* Enable RX interrupt */
    byte = 0x01;
    __asm__("out %0, %1" : : "a"(byte), "d"(UART_IER));
    
    return 1;
}

/**
 * @brief Send byte via serial port
 */
int serial_send_byte(uint8_t byte) {
    /* Wait for TX holding register empty */
    uint8_t lsr;
    int timeout = 0;
    
    do {
        __asm__("in %1, %0" : "=a"(lsr) : "d"(UART_LSR));
        timeout++;
    } while (!(lsr & LSR_THRE) && timeout < 100000);
    
    if (timeout >= 100000) {
        return -1;  /* Timeout */
    }
    
    /* Send byte */
    __asm__("out %0, %1" : : "a"(byte), "d"(UART_DATA));
    
    return 0;
}

/**
 * @brief Send string via serial port
 */
int serial_send_str(const char *str) {
    if (!str) return -1;
    
    while (*str) {
        if (serial_send_byte(*str)) {
            return -1;
        }
        str++;
    }
    
    return 0;
}

/**
 * @brief Receive byte from serial port
 */
uint8_t serial_recv_byte(void) {
    uint8_t lsr;
    
    /* Wait for data ready */
    do {
        __asm__("in %1, %0" : "=a"(lsr) : "d"(UART_LSR));
    } while (!(lsr & LSR_DR));
    
    /* Read byte */
    uint8_t byte;
    __asm__("in %1, %0" : "=a"(byte) : "d"(UART_DATA));
    
    return byte;
}

/**
 * @brief Check if data available
 */
int serial_data_available(void) {
    uint8_t lsr;
    __asm__("in %1, %0" : "=a"(lsr) : "d"(UART_LSR));
    return (lsr & LSR_DR) != 0;
}

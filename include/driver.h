/**
 * @file driver.h
 * @brief Device driver interface
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Defines interfaces for VGA display, PS/2 keyboard, and serial port drivers.
 * All hardware I/O is abstracted through these interfaces.
 */

#ifndef DRIVER_H
#define DRIVER_H

#include "types.h"

/* ==================== VGA TEXT MODE DRIVER ==================== */

/**
 * @brief Initialize VGA text mode driver
 * 
 * Sets up 80x25 text mode with memory-mapped I/O at 0xB8000.
 * Clears screen and sets initial position to (0, 0).
 * 
 * @return 0 on success, -1 on failure
 */
int vga_init(void);

/**
 * @brief Write character to screen at current cursor position
 * 
 * @param ch Character to write
 * @param color Color attribute (foreground in lower 4 bits, background in upper 4 bits)
 */
void vga_putchar(char ch, uint8_t color);

/**
 * @brief Write string to screen
 * 
 * @param str String to write
 * @param color Color attribute
 */
void vga_putstr(const char *str, uint8_t color);

/**
 * @brief Move cursor to specified position
 * 
 * @param x Column (0-79)
 * @param y Row (0-24)
 */
void vga_move_cursor(uint8_t x, uint8_t y);

/**
 * @brief Get current cursor position
 * 
 * @param x Output: column
 * @param y Output: row
 */
void vga_get_cursor(uint8_t *x, uint8_t *y);

/**
 * @brief Clear screen and reset cursor
 */
void vga_clear(void);

/**
 * @brief Scroll screen up by one line
 */
void vga_scroll_up(void);

/* VGA Color constants */
#define VGA_COLOR_BLACK         0x0
#define VGA_COLOR_BLUE          0x1
#define VGA_COLOR_GREEN         0x2
#define VGA_COLOR_CYAN          0x3
#define VGA_COLOR_RED           0x4
#define VGA_COLOR_MAGENTA       0x5
#define VGA_COLOR_BROWN         0x6
#define VGA_COLOR_LIGHT_GRAY    0x7
#define VGA_COLOR_DARK_GRAY     0x8
#define VGA_COLOR_LIGHT_BLUE    0x9
#define VGA_COLOR_LIGHT_GREEN   0xA
#define VGA_COLOR_LIGHT_CYAN    0xB
#define VGA_COLOR_LIGHT_RED     0xC
#define VGA_COLOR_LIGHT_MAGENTA 0xD
#define VGA_COLOR_YELLOW        0xE
#define VGA_COLOR_WHITE         0xF

#define VGA_COLOR_BG_SHIFT      4
#define VGA_COLOR(fg, bg)       ((bg << VGA_COLOR_BG_SHIFT) | fg)

/* ==================== PS/2 KEYBOARD DRIVER ==================== */

/**
 * @brief Initialize PS/2 keyboard driver
 * 
 * Initializes IRQ1 handler and sets up scancode-to-ASCII translation.
 * Keyboard input is buffered and can be read with kbd_getchar().
 * 
 * @return 0 on success, -1 on failure
 */
int kbd_init(void);

/**
 * @brief Read character from keyboard input buffer
 * 
 * Blocks until a character is available (or returns 0 if non-blocking).
 * Translates scancodes to ASCII characters.
 * 
 * @return ASCII character, 0 if no input available
 */
char kbd_getchar(void);

/**
 * @brief Check if character available in keyboard buffer
 * @return 1 if character available, 0 otherwise
 */
int kbd_char_available(void);

/**
 * @brief Handle special keys (Shift, Ctrl, Alt, etc.)
 * 
 * Internal function called from keyboard IRQ handler.
 * Updates shift state and modifiers.
 * 
 * @param scancode Keyboard scancode
 */
void kbd_handle_special_key(uint8_t scancode);

/* ==================== SERIAL PORT DRIVER ==================== */

/**
 * @brief Initialize serial port (COM1)
 * 
 * Initializes UART at 115200 baud, 8N1 format.
 * Used for debug output via serial console.
 * 
 * @return 0 on success, -1 on failure
 */
int serial_init(void);

/**
 * @brief Send byte via serial port
 * 
 * @param byte Byte to send
 * @return 0 on success, -1 on failure
 */
int serial_send_byte(uint8_t byte);

/**
 * @brief Send string via serial port
 * 
 * @param str String to send
 * @return 0 on success, -1 on failure
 */
int serial_send_str(const char *str);

/**
 * @brief Receive byte from serial port
 * 
 * Blocks until byte is received.
 * 
 * @return Received byte
 */
uint8_t serial_recv_byte(void);

/**
 * @brief Check if data available in serial receive buffer
 * @return 1 if data available, 0 otherwise
 */
int serial_data_available(void);

/* Serial port constants */
#define COM1_PORT           0x3F8
#define SERIAL_BAUD_RATE    115200

#endif /* DRIVER_H */

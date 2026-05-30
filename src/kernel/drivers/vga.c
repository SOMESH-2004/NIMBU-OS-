/**
 * @file vga.c
 * @brief VGA text mode driver implementation
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Implements 80x25 text mode display driver using memory-mapped I/O.
 */

#include "driver.h"
#include "interrupt.h"

#define VGA_BUFFER      0xB8000
#define VGA_WIDTH       80
#define VGA_HEIGHT      25

static struct {
    uint16_t *buffer;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t current_color;
} vga_state = {(uint16_t *)VGA_BUFFER, 0, 0, 7};

/**
 * @brief Update hardware cursor position
 */
static void update_hardware_cursor(void) {
    uint16_t pos = vga_state.cursor_y * VGA_WIDTH + vga_state.cursor_x;
    
    __asm__("mov $0x0E, %%al; out %%al, $0x3D4; mov %0, %%al; out %%al, $0x3D5"
            : : "r"((uint8_t)(pos >> 8)));
    __asm__("mov $0x0F, %%al; out %%al, $0x3D4; mov %0, %%al; out %%al, $0x3D5"
            : : "r"((uint8_t)(pos & 0xFF)));
}

/**
 * @brief Initialize VGA driver
 */
int vga_init(void) {
    vga_clear();
    return 1;
}

/**
 * @brief Write character at cursor position
 */
void vga_putchar(char ch, uint8_t color) {
    if (ch == '\n') {
        vga_state.cursor_y++;
        vga_state.cursor_x = 0;
        
        if (vga_state.cursor_y >= VGA_HEIGHT) {
            vga_scroll_up();
            vga_state.cursor_y = VGA_HEIGHT - 1;
        }
    } else {
        uint16_t index = vga_state.cursor_y * VGA_WIDTH + vga_state.cursor_x;
        vga_state.buffer[index] = ((uint16_t)color << 8) | ch;
        
        vga_state.cursor_x++;
        if (vga_state.cursor_x >= VGA_WIDTH) {
            vga_state.cursor_x = 0;
            vga_state.cursor_y++;
            
            if (vga_state.cursor_y >= VGA_HEIGHT) {
                vga_scroll_up();
                vga_state.cursor_y = VGA_HEIGHT - 1;
            }
        }
    }
    
    update_hardware_cursor();
}

/**
 * @brief Write string
 */
void vga_putstr(const char *str, uint8_t color) {
    if (!str) return;
    
    while (*str) {
        vga_putchar(*str, color);
        str++;
    }
}

/**
 * @brief Move cursor
 */
void vga_move_cursor(uint8_t x, uint8_t y) {
    if (x >= VGA_WIDTH) x = VGA_WIDTH - 1;
    if (y >= VGA_HEIGHT) y = VGA_HEIGHT - 1;
    
    vga_state.cursor_x = x;
    vga_state.cursor_y = y;
    update_hardware_cursor();
}

/**
 * @brief Get cursor position
 */
void vga_get_cursor(uint8_t *x, uint8_t *y) {
    if (x) *x = vga_state.cursor_x;
    if (y) *y = vga_state.cursor_y;
}

/**
 * @brief Clear screen
 */
void vga_clear(void) {
    uint16_t blank = ((uint16_t)(VGA_COLOR_BLACK << VGA_COLOR_BG_SHIFT | VGA_COLOR_WHITE)) << 8;
    blank |= ' ';
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_state.buffer[i] = blank;
    }
    
    vga_state.cursor_x = 0;
    vga_state.cursor_y = 0;
    update_hardware_cursor();
}

/**
 * @brief Scroll up by one line
 */
void vga_scroll_up(void) {
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            uint16_t index = y * VGA_WIDTH + x;
            uint16_t next_index = (y + 1) * VGA_WIDTH + x;
            vga_state.buffer[index] = vga_state.buffer[next_index];
        }
    }
    
    /* Clear last line */
    uint16_t blank = ((uint16_t)(VGA_COLOR_BLACK << VGA_COLOR_BG_SHIFT | VGA_COLOR_WHITE)) << 8;
    blank |= ' ';
    
    for (int x = 0; x < VGA_WIDTH; x++) {
        uint16_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        vga_state.buffer[index] = blank;
    }
}

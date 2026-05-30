/**
 * @file kbd.c
 * @brief PS/2 keyboard driver implementation
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Implements keyboard input handling with scancode-to-ASCII translation.
 */

#include "driver.h"
#include "interrupt.h"

#define KBD_BUFFER_SIZE 256
#define KBD_DATA_PORT   0x60
#define KBD_CMD_PORT    0x64

/* Keyboard input buffer */
static struct {
    char buffer[KBD_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint8_t shift_pressed;
    uint8_t ctrl_pressed;
} kbd_state = {{0}, 0, 0, 0, 0};

/* Scancode to ASCII translation table */
static const char scancode_table[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/**
 * @brief Keyboard interrupt handler
 */
static void kbd_interrupt_handler(interrupt_context_t *context) {
    /* Read scancode */
    uint8_t scancode;
    __asm__("in $0x60, %%al" : "=a"(scancode));
    
    /* Handle key release (high bit set) */
    if (scancode & 0x80) {
        scancode &= 0x7F;
        
        /* Check for shift/ctrl release */
        if (scancode == 0x2A || scancode == 0x36) {  /* Shift */
            kbd_state.shift_pressed = 0;
        }
        if (scancode == 0x1D) {  /* Ctrl */
            kbd_state.ctrl_pressed = 0;
        }
    } else {
        /* Key press */
        kbd_handle_special_key(scancode);
        
        if (scancode < 128 && scancode_table[scancode]) {
            char ch = scancode_table[scancode];
            
            if (kbd_state.shift_pressed) {
                if (ch >= 'a' && ch <= 'z') {
                    ch = ch - 'a' + 'A';
                }
            }
            
            /* Add to buffer */
            uint16_t next = (kbd_state.tail + 1) % KBD_BUFFER_SIZE;
            if (next != kbd_state.head) {
                kbd_state.buffer[kbd_state.tail] = ch;
                kbd_state.tail = next;
            }
        }
    }
    
    /* Acknowledge interrupt */
    __asm__("mov $0x20, %%al; out %%al, $0x20");
}

/**
 * @brief Initialize keyboard driver
 */
int kbd_init(void) {
    kbd_state.head = 0;
    kbd_state.tail = 0;
    kbd_state.shift_pressed = 0;
    kbd_state.ctrl_pressed = 0;
    
    /* Register interrupt handler for IRQ1 (keyboard) */
    idt_register_handler(33, kbd_interrupt_handler);
    
    /* Enable keyboard IRQ */
    irq_enable(1);
    
    return 1;
}

/**
 * @brief Get character from keyboard buffer
 */
char kbd_getchar(void) {
    if (kbd_state.head == kbd_state.tail) {
        return 0;  /* No character available */
    }
    
    char ch = kbd_state.buffer[kbd_state.head];
    kbd_state.head = (kbd_state.head + 1) % KBD_BUFFER_SIZE;
    
    return ch;
}

/**
 * @brief Check if character available
 */
int kbd_char_available(void) {
    return kbd_state.head != kbd_state.tail;
}

/**
 * @brief Handle special keys
 */
void kbd_handle_special_key(uint8_t scancode) {
    if (scancode == 0x2A || scancode == 0x36) {  /* Shift */
        kbd_state.shift_pressed = 1;
    }
    if (scancode == 0x1D) {  /* Ctrl */
        kbd_state.ctrl_pressed = 1;
    }
}

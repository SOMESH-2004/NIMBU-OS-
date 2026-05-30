/**
 * @file idt.c
 * @brief Interrupt Descriptor Table implementation (x86)
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Implements IDT setup with ISR and IRQ handlers.
 */

#include "interrupt.h"

/* IDT entry structure */
typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

/* IDT pointer structure */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[256];
static idt_ptr_t idt_ptr;
static interrupt_handler_t interrupt_handlers[256];

/**
 * @brief External ISR stubs (defined in ISR assembly file)
 */
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

/**
 * @brief Set IDT entry
 */
static void idt_set_entry(uint8_t vector, uint32_t handler, uint16_t selector, uint8_t flags) {
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].flags = flags;
    idt[vector].reserved = 0;
}

/**
 * @brief Initialize 8259 PIC (Programmable Interrupt Controller)
 */
static void pic_init(void) {
    /* ICW1 - Edge triggered, single, ICW4 needed */
    __asm__("mov $0x20, %%al; out %%al, $0x20" : :);
    __asm__("mov $0x20, %%al; out %%al, $0xA0" : :);
    
    /* ICW2 - Remap IRQ to vectors 32-47 */
    __asm__("mov $0x08, %%al; out %%al, $0x21" : :);
    __asm__("mov $0x70, %%al; out %%al, $0xA1" : :);
    
    /* ICW3 - Cascade mode */
    __asm__("mov $0x04, %%al; out %%al, $0x21" : :);
    __asm__("mov $0x02, %%al; out %%al, $0xA1" : :);
    
    /* ICW4 - 8086 mode */
    __asm__("mov $0x01, %%al; out %%al, $0x21" : :);
    __asm__("mov $0x01, %%al; out %%al, $0xA1" : :);
    
    /* OCW1 - Mask interrupts (will be unmasked individually) */
    __asm__("mov $0xFF, %%al; out %%al, $0x21" : :);
    __asm__("mov $0xFF, %%al; out %%al, $0xA1" : :);
}

/**
 * @brief Initialize IDT
 */
int idt_init(void) {
    /* Set IDT pointer */
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;
    
    /* Set all handlers to default */
    for (int i = 0; i < 256; i++) {
        interrupt_handlers[i] = isr_default_handler;
    }
    
    /* ISR entries (CPU exceptions) */
    idt_set_entry(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_entry(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_entry(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_entry(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_entry(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_entry(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_entry(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_entry(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_entry(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_entry(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_entry(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_entry(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_entry(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_entry(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_entry(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_entry(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_entry(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_entry(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_entry(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_entry(19, (uint32_t)isr19, 0x08, 0x8E);
    
    /* IRQ entries (hardware interrupts) */
    idt_set_entry(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_entry(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_entry(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_entry(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_entry(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_entry(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_entry(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_entry(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_entry(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_entry(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_entry(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_entry(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_entry(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_entry(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_entry(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_entry(47, (uint32_t)irq15, 0x08, 0x8E);
    
    /* Page fault handler */
    interrupt_handlers[INT_PAGE_FAULT] = isr_page_fault_handler;
    
    /* Initialize PIC */
    pic_init();
    
    /* Load IDT */
    __asm__("lidt %0" : : "m"(idt_ptr));
    
    return 1;
}

/**
 * @brief Register interrupt handler
 */
int idt_register_handler(uint8_t vector, interrupt_handler_t handler) {
    if (vector >= 256) {
        return -1;
    }
    interrupt_handlers[vector] = handler;
    return 0;
}

/**
 * @brief Default exception handler
 */
void isr_default_handler(interrupt_context_t *context) {
    vga_putstr("EXCEPTION: INT ", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    /* Print exception number */
    char buf[8];
    /* Simple number to string (for demo) */
    vga_putstr("\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    
    /* Halt system */
    while (1);
}

/**
 * @brief Page fault handler
 */
void isr_page_fault_handler(interrupt_context_t *context) {
    vga_putstr("PAGE FAULT at EIP: ", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    /* Print address and halt */
    while (1);
}

/**
 * @brief Enable interrupts
 */
void interrupts_enable(void) {
    __asm__("sti");
}

/**
 * @brief Disable interrupts
 */
void interrupts_disable(void) {
    __asm__("cli");
}

/**
 * @brief Enable IRQ
 */
void irq_enable(uint8_t irq) {
    uint16_t port = 0x21;
    if (irq >= 8) {
        port = 0xA1;
        irq -= 8;
    }
    
    uint8_t mask;
    __asm__("in %1, %0" : "=a"(mask) : "d"(port));
    mask &= ~(1 << irq);
    __asm__("out %0, %1" : : "a"(mask), "d"(port));
}

/**
 * @brief Disable IRQ
 */
void irq_disable(uint8_t irq) {
    uint16_t port = 0x21;
    if (irq >= 8) {
        port = 0xA1;
        irq -= 8;
    }
    
    uint8_t mask;
    __asm__("in %1, %0" : "=a"(mask) : "d"(port));
    mask |= (1 << irq);
    __asm__("out %0, %1" : : "a"(mask), "d"(port));
}

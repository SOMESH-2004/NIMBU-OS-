/**
 * @file interrupt.h
 * @brief Interrupt handling interface
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Defines interrupt handling mechanisms including IDT setup, ISRs, and IRQs.
 * Supports 256 interrupt vectors: 0-31 (CPU exceptions), 32-47 (hardware IRQs),
 * 128 (syscall).
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "types.h"

/* Interrupt types */
#define INT_CPU_EXCEPTION   0   /* 0-31: CPU exceptions */
#define INT_HARDWARE_IRQ    32  /* 32-47: Hardware interrupts */
#define INT_SYSCALL         128 /* Software interrupt for syscalls */

/**
 * @brief Interrupt context structure
 * 
 * Captures all CPU registers at the time of interrupt.
 * Used for context switching and exception handling.
 */
typedef struct {
    uint32_t gs, fs, es, ds;            /* Segment registers */
    uint32_t edi, esi, ebp, esp;        /* General purpose registers */
    uint32_t ebx, edx, ecx, eax;
    uint32_t int_no, err_code;          /* Interrupt number and error code */
    uint32_t eip, cs, eflags;           /* Return address and flags */
    uint32_t user_esp, ss;              /* User stack pointer (ring 3) */
} interrupt_context_t;

/**
 * @brief Interrupt handler callback type
 * 
 * @param context Pointer to interrupt context with register values
 */
typedef void (*interrupt_handler_t)(interrupt_context_t *context);

/**
 * @brief Initialize Interrupt Descriptor Table
 * 
 * Sets up IDT with 256 interrupt vectors. Maps CPU exceptions to ISRs
 * and hardware interrupts to IRQ handlers. Initializes 8259 PIC.
 * 
 * @return 0 on success, -1 on failure
 */
int idt_init(void);

/**
 * @brief Register interrupt handler
 * 
 * Registers a custom handler for a specific interrupt vector.
 * 
 * @param vector Interrupt vector (0-255)
 * @param handler Handler function pointer
 * @return 0 on success, -1 on failure
 */
int idt_register_handler(uint8_t vector, interrupt_handler_t handler);

/**
 * @brief Default exception handler
 * 
 * Handles CPU exceptions (divide-by-zero, page fault, GPF, etc.)
 * Prints exception information and halts.
 * 
 * @param context Interrupt context
 */
void isr_default_handler(interrupt_context_t *context);

/**
 * @brief Page fault handler
 * 
 * Handles page faults (INT 14). Attempts to handle demand paging.
 * 
 * @param context Interrupt context
 */
void isr_page_fault_handler(interrupt_context_t *context);

/**
 * @brief Enable all interrupts
 */
void interrupts_enable(void);

/**
 * @brief Disable all interrupts
 */
void interrupts_disable(void);

/**
 * @brief Enable specific IRQ
 * @param irq IRQ number (0-15)
 */
void irq_enable(uint8_t irq);

/**
 * @brief Disable specific IRQ
 * @param irq IRQ number (0-15)
 */
void irq_disable(uint8_t irq);

/* Exception codes */
#define INT_DIVIDE_BY_ZERO  0
#define INT_DEBUG           1
#define INT_NMI             2
#define INT_BREAKPOINT      3
#define INT_OVERFLOW        4
#define INT_BOUND_EXCEED    5
#define INT_INVALID_OP      6
#define INT_DEVICE_NOT_AV   7
#define INT_DOUBLE_FAULT    8
#define INT_INVALID_TSS     10
#define INT_SEGMENT_NOT_P   11
#define INT_STACK_FAULT     12
#define INT_GENERAL_PROT    13
#define INT_PAGE_FAULT      14
#define INT_FPU_EXCEPTION   16
#define INT_ALIGNMENT_CHK   17

/* Hardware IRQ numbers (after remapping) */
#define IRQ_TIMER           0   /* PIT timer */
#define IRQ_KEYBOARD        1   /* PS/2 keyboard */
#define IRQ_CASCADE         2   /* Slave PIC */
#define IRQ_SERIAL_2        3   /* COM2/COM4 */
#define IRQ_SERIAL_1        4   /* COM1/COM3 */
#define IRQ_PARALLEL_2      5   /* LPT2 */
#define IRQ_FLOPPY          6   /* Floppy disk */
#define IRQ_PARALLEL_1      7   /* LPT1 */
#define IRQ_RTC             8   /* Real-time clock */
#define IRQ_ACPI            9
#define IRQ_RESERVED_1      10
#define IRQ_RESERVED_2      11
#define IRQ_MOUSE           12  /* PS/2 mouse */
#define IRQ_COPROCESSOR     13  /* FPU */
#define IRQ_HDD_1           14  /* IDE primary */
#define IRQ_HDD_2           15  /* IDE secondary */

#endif /* INTERRUPT_H */

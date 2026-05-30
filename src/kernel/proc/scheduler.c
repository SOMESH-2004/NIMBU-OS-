/**
 * @file scheduler.c
 * @brief Process scheduler implementation
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Implements round-robin preemptive scheduler with PIT timer interrupt.
 * Timer fires at 100 Hz for context switching.
 */

#include "process.h"
#include "driver.h"

#define PIT_PORT_CMD    0x43
#define PIT_PORT_DATA   0x40
#define PIT_FREQUENCY   1193182
#define PIT_DIVISOR     (PIT_FREQUENCY / 100)  /* 100 Hz */

static uint32_t scheduler_ticks = 0;

/**
 * @brief PIT timer interrupt handler (stub - will be implemented)
 */
extern void pit_handler(interrupt_context_t *context);

/**
 * @brief Initialize scheduler
 */
int scheduler_init(void) {
    /* Set up PIT timer for 100 Hz */
    uint8_t cmd = 0x34;  /* Counter 0, load LSB then MSB, mode 2 (rate generator) */
    __asm__("out %0, %1" : : "a"(cmd), "d"(PIT_PORT_CMD));
    
    uint8_t divisor_low = PIT_DIVISOR & 0xFF;
    uint8_t divisor_high = (PIT_DIVISOR >> 8) & 0xFF;
    
    __asm__("out %0, %1" : : "a"(divisor_low), "d"(PIT_PORT_DATA));
    __asm__("out %0, %1" : : "a"(divisor_high), "d"(PIT_PORT_DATA));
    
    /* Register timer handler for IRQ0 */
    idt_register_handler(32, scheduler_tick);
    
    /* Enable timer IRQ */
    irq_enable(0);
    
    return 1;
}

/**
 * @brief Scheduler tick - called from PIT interrupt
 */
void scheduler_tick(interrupt_context_t *context) {
    scheduler_ticks++;
    
    pcb_t *current = proc_get_current();
    if (!current) {
        return;
    }
    
    /* Decrement time slice */
    if (current->time_slice_remaining > 0) {
        current->time_slice_remaining--;
    }
    
    /* Check if time slice expired */
    if (current->time_slice_remaining == 0 || current->state != PROC_STATE_RUNNING) {
        /* Need context switch */
        context_switch(context);
    }
    
    /* Send EOI to PIC */
    __asm__("mov $0x20, %%al; out %%al, $0x20");
}

/**
 * @brief Context switch - find next ready process
 */
interrupt_context_t *context_switch(interrupt_context_t *context) {
    pcb_t *current = proc_get_current();
    pcb_t *next = NULL;
    
    /* Find first ready process */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROC_STATE_READY) {
            next = &process_table[i];
            break;
        }
    }
    
    /* If no ready process, idle */
    if (!next) {
        next = &process_table[0];  /* Idle process */
    }
    
    /* Switch states */
    if (current) {
        current->state = PROC_STATE_READY;
        current->context = context;
    }
    
    next->state = PROC_STATE_RUNNING;
    next->time_slice_remaining = SCHEDULER_TIME_SLICE;
    next->total_runtime++;
    
    /* Update current process pointer */
    /* This is set elsewhere in the system */
    
    return next->context ? next->context : context;
}

/**
 * @brief Get scheduler ticks
 */
uint32_t scheduler_get_ticks(void) {
    return scheduler_ticks;
}

/**
 * @file process.c
 * @brief Process management implementation
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Implements process control blocks and basic process management.
 */

#include "process.h"
#include "mem.h"
#include "driver.h"

/* Process table */
static pcb_t process_table[MAX_PROCESSES];
static uint32_t current_pid = 0;
static pcb_t *current_process = NULL;
static uint32_t next_pid = 1;

/* Queue heads */
static pcb_t *ready_queue = NULL;
static pcb_t *running_process = NULL;

/**
 * @brief Initialize process management
 */
int proc_init(void) {
    /* Initialize process table */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROC_STATE_READY;
        process_table[i].context = NULL;
    }
    
    /* Create idle process (PID 0) */
    process_table[0].pid = 0;
    process_table[0].state = PROC_STATE_RUNNING;
    current_process = &process_table[0];
    current_pid = 0;
    
    return 1;
}

/**
 * @brief Create new process
 */
int proc_create(void *entry_point, uint32_t priority) {
    if (next_pid >= MAX_PROCESSES) {
        return -1;  /* No more PIDs */
    }
    
    pcb_t *pcb = &process_table[next_pid];
    pcb->pid = next_pid;
    pcb->parent_pid = current_pid;
    pcb->state = PROC_STATE_READY;
    pcb->priority = priority;
    pcb->time_slice_remaining = SCHEDULER_TIME_SLICE;
    pcb->total_runtime = 0;
    
    /* Allocate stack and page directory */
    phys_addr_t stack_phys = pmm_alloc();
    phys_addr_t pd_phys = pmm_alloc();
    
    if (stack_phys == 0 || pd_phys == 0) {
        return -1;  /* Out of memory */
    }
    
    pcb->kernel_stack = KERNEL_VIRT_BASE + 0x300000 + (next_pid * PAGE_SIZE);
    pcb->page_dir_phys = pd_phys;
    
    /* Allocate and initialize interrupt context */
    pcb->context = (interrupt_context_t *)kmalloc(sizeof(interrupt_context_t));
    if (!pcb->context) {
        return -1;
    }
    
    /* Initialize context with entry point */
    pcb->context->eip = (uint32_t)entry_point;
    pcb->context->cs = 0x08;      /* Kernel code segment */
    pcb->context->ds = 0x10;      /* Kernel data segment */
    pcb->context->es = 0x10;
    pcb->context->fs = 0x10;
    pcb->context->gs = 0x10;
    pcb->context->ss = 0x10;
    pcb->context->esp = pcb->kernel_stack + PAGE_SIZE - 4;
    pcb->context->eflags = 0x202;  /* IF set */
    
    next_pid++;
    
    return pcb->pid;
}

/**
 * @brief Exit process
 */
int proc_exit(uint32_t pid) {
    if (pid >= MAX_PROCESSES) {
        return -1;
    }
    
    process_table[pid].state = PROC_STATE_ZOMBIE;
    
    if (pid == current_pid) {
        /* Switch to another process */
        /* This will be handled by scheduler */
    }
    
    return 0;
}

/**
 * @brief Get current process
 */
pcb_t *proc_get_current(void) {
    return current_process;
}

/**
 * @brief Get process by PID
 */
pcb_t *proc_get(uint32_t pid) {
    if (pid >= MAX_PROCESSES) {
        return NULL;
    }
    return &process_table[pid];
}

/**
 * @brief Block current process
 */
int proc_block(void *resource) {
    if (!current_process) {
        return -1;
    }
    
    current_process->state = PROC_STATE_BLOCKED;
    current_process->blocked_on = resource;
    
    return 0;
}

/**
 * @brief Unblock process
 */
int proc_unblock(uint32_t pid) {
    if (pid >= MAX_PROCESSES) {
        return -1;
    }
    
    if (process_table[pid].state != PROC_STATE_BLOCKED) {
        return -1;
    }
    
    process_table[pid].state = PROC_STATE_READY;
    process_table[pid].blocked_on = NULL;
    
    return 0;
}

/**
 * @brief Halt all processes
 */
void proc_halt_all(void) {
    interrupts_disable();
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].state = PROC_STATE_ZOMBIE;
    }
    
    while (1) {
        __asm__("hlt");
    }
}

/**
 * @brief Get scheduler statistics
 */
int scheduler_get_stats(uint32_t *ready_count, uint32_t *running_count, uint32_t *blocked_count) {
    uint32_t ready = 0, running = 0, blocked = 0;
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        switch (process_table[i].state) {
            case PROC_STATE_READY:
                ready++;
                break;
            case PROC_STATE_RUNNING:
                running++;
                break;
            case PROC_STATE_BLOCKED:
                blocked++;
                break;
        }
    }
    
    if (ready_count) *ready_count = ready;
    if (running_count) *running_count = running;
    if (blocked_count) *blocked_count = blocked;
    
    return 0;
}

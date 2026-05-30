/**
 * @file process.h
 * @brief Process and scheduling interface
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Defines process management including Process Control Blocks (PCBs),
 * scheduler, and context switching. Uses round-robin preemptive scheduling
 * with timer-driven context switches at 100 Hz.
 */

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "interrupt.h"

/* Process states */
#define PROC_STATE_READY    0
#define PROC_STATE_RUNNING  1
#define PROC_STATE_BLOCKED  2
#define PROC_STATE_ZOMBIE   3

/* Process priority levels (not fully utilized in basic round-robin) */
#define PROC_PRIORITY_LOW       2
#define PROC_PRIORITY_NORMAL    1
#define PROC_PRIORITY_HIGH      0

/* Maximum number of processes */
#define MAX_PROCESSES   256

/* Time slice for round-robin scheduling (ticks) */
#define SCHEDULER_TIME_SLICE    10  /* 100 ms at 100 Hz */

/**
 * @brief Process Control Block
 * 
 * Contains all state information for a process including registers,
 * memory mapping, and scheduling state.
 */
typedef struct {
    uint32_t pid;                       /* Process ID */
    uint32_t parent_pid;                /* Parent process ID */
    uint32_t state;                     /* READY, RUNNING, BLOCKED, ZOMBIE */
    uint32_t priority;                  /* Process priority */
    
    /* CPU registers */
    interrupt_context_t *context;       /* Saved register context */
    
    /* Memory management */
    uint32_t page_dir_phys;             /* Physical address of page directory */
    uint32_t kernel_stack;              /* Kernel stack pointer */
    uint32_t user_stack;                /* User stack pointer */
    
    /* Scheduling info */
    uint32_t time_slice_remaining;      /* Remaining ticks in current slice */
    uint32_t total_runtime;             /* Total CPU time used (ticks) */
    uint32_t wait_queue;                /* Which queue process is on (if blocked) */
    
    /* IPC / Synchronization */
    void *blocked_on;                   /* Resource being waited for */
} pcb_t;

/**
 * @brief Initialize process management
 * 
 * Initializes process tables and scheduler. Must be called early.
 * 
 * @return 0 on success, -1 on failure
 */
int proc_init(void);

/**
 * @brief Create a new process
 * 
 * Allocates and initializes a new process structure with page directory
 * and stacks.
 * 
 * @param entry_point Entry point function of process
 * @param priority Process priority
 * @return PID of new process, -1 on failure
 */
int proc_create(void *entry_point, uint32_t priority);

/**
 * @brief Terminate a process
 * 
 * Marks process as ZOMBIE and reclaims resources.
 * 
 * @param pid Process ID to terminate
 * @return 0 on success, -1 on failure
 */
int proc_exit(uint32_t pid);

/**
 * @brief Get current process PCB
 * @return Pointer to current process PCB
 */
pcb_t *proc_get_current(void);

/**
 * @brief Get process by PID
 * @param pid Process ID
 * @return Pointer to PCB, NULL if not found
 */
pcb_t *proc_get(uint32_t pid);

/**
 * @brief Initialize scheduler
 * 
 * Sets up round-robin scheduler with timer interrupt at 100 Hz.
 * 
 * @return 0 on success, -1 on failure
 */
int scheduler_init(void);

/**
 * @brief Scheduler tick - called on every timer interrupt
 * 
 * Updates scheduler state and may trigger context switch.
 * Called from PIT timer IRQ handler.
 * 
 * @param context Current interrupt context (saved registers)
 */
void scheduler_tick(interrupt_context_t *context);

/**
 * @brief Perform context switch
 * 
 * Saves current process state and loads next ready process.
 * 
 * @param context Current interrupt context
 * @return New context to restore (may be different process)
 */
interrupt_context_t *context_switch(interrupt_context_t *context);

/**
 * @brief Block current process
 * 
 * Removes process from READY queue and marks as BLOCKED.
 * Scheduler will not run this process until unblocked.
 * 
 * @param resource Resource being waited for (for debugging)
 * @return 0 on success, -1 on failure
 */
int proc_block(void *resource);

/**
 * @brief Unblock a process
 * 
 * Moves process from BLOCKED to READY queue.
 * 
 * @param pid Process ID to unblock
 * @return 0 on success, -1 on failure
 */
int proc_unblock(uint32_t pid);

/**
 * @brief Get scheduler statistics
 * @param ready_count Output: number of ready processes
 * @param running_count Output: number of running processes
 * @param blocked_count Output: number of blocked processes
 * @return 0 on success, -1 on failure
 */
int scheduler_get_stats(uint32_t *ready_count, uint32_t *running_count, uint32_t *blocked_count);

/**
 * @brief Kill all processes and halt system
 */
void proc_halt_all(void);

#endif /* PROCESS_H */

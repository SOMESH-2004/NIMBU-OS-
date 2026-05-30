# NIMBU OS - Source Code Implementation Guide

**Detailed Walkthrough of Key Implementation Files**

---

## Table of Contents

1. [Bootloader (boot.asm)](#bootloader-bootasm)
2. [Kernel Initialization (kernel.c)](#kernel-initialization-kernelc)
3. [GDT Implementation (gdt.c)](#gdt-implementation-gdtc)
4. [IDT & Interrupts (idt.c)](#idt--interrupts-idtc)
5. [Physical Memory Manager (pmm.c)](#physical-memory-manager-pmmc)
6. [Virtual Memory Manager (vmm.c)](#virtual-memory-manager-vmmc)
7. [Process Management (process.c)](#process-management-processc)
8. [Scheduler (scheduler.c)](#scheduler-schedulerc)
9. [VGA Driver (vga.c)](#vga-driver-vgac)
10. [Keyboard Driver (kbd.c)](#keyboard-driver-kbdc)
11. [Shell Implementation (shell.c)](#shell-implementation-shellc)

---

## Bootloader (boot.asm)

**Location**: [src/boot/boot.asm](src/boot/boot.asm)

The bootloader is the first code executed after BIOS loads it. It's responsible for initial CPU setup and jumping to kernel.

### Key Components

#### Multiboot Header
```asm
; Multiboot magic number and flags
MULTIBOOT_MAGIC     EQU 0x1BADB002
MULTIBOOT_FLAGS     EQU 0x00000001
MULTIBOOT_CHECKSUM  EQU -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

DD MULTIBOOT_MAGIC
DD MULTIBOOT_FLAGS
DD MULTIBOOT_CHECKSUM
```

**Purpose**: Tells GRUB bootloader this is a valid kernel image.

**Structure**:
- Magic number identifies bootloader protocol
- Flags specify optional features
- Checksum validates magic+flags (sum must = 0 mod 2^32)

#### Stack Initialization
```asm
ALIGN 4096
kernel_stack:
    RESB 8192           ; Reserve 8 KB for stack

_start:
    MOV ESP, kernel_stack + 8192
```

**Purpose**: Set up stack pointer.

**Why stack grows down**: x86 convention - ESP points to top of stack, decreases as data is pushed.

**Size**: 8 KB (8192 bytes) - adequate for kernel operations.

#### CPU Setup
```asm
CLI                     ; Disable interrupts (IF flag in EFLAGS)
XOR EAX, EAX
PUSH EAX
POPFD                   ; Clear EFLAGS register
```

**Purpose**: 
- CLI disables hardware interrupts
- Clearing EFLAGS removes any flags (DF for string operations, etc.)

**Why important**: Ensures clean state before kernel runs.

#### Kernel Call
```asm
CALL kmain
CLI
HLT
JMP $
```

**Purpose**:
- CALL kmain: Jump to kernel initialization
- CLI: Disable interrupts
- HLT: Stop CPU
- JMP $: Infinite loop at same address

**Never returns**: If kmain returns, system halts. This is the expected behavior.

### Execution Flow

```
BIOS Bootloader
      ↓
   boot.asm (_start)
      ├─ Set ESP to kernel_stack + 8192
      ├─ CLI (disable interrupts)
      ├─ Clear EFLAGS
      └─ CALL kmain
```

---

## Kernel Initialization (kernel.c)

**Location**: [src/kernel/kernel.c](src/kernel/kernel.c)

The main kernel entry point after bootloader. Orchestrates initialization of all subsystems.

### Initialization Order

```c
void kmain(void) {
    vga_init();             /* 1. Display output first */
    print_banner();
    
    gdt_init();             /* 2. CPU structures */
    idt_init();
    
    pmm_init(...);          /* 3. Memory management */
    vmm_init();
    heap_init();
    
    idt_setup_handlers();   /* 4. Interrupt handlers */
    kbd_init();             /* 5. Device drivers */
    serial_init();
    timer_init();
    
    proc_init();            /* 6. Process management */
    scheduler_init();
    
    shell_run();            /* 7. Shell (never returns) */
}
```

### Why This Order?

1. **VGA first**: Need console output for debugging
2. **GDT/IDT**: Must be ready before interrupts
3. **Memory**: Required for heap allocation
4. **Interrupts**: Must be set up before I/O
5. **Drivers**: Initialize device hardware
6. **Processes/Scheduler**: Start multitasking
7. **Shell**: User interface

### Key Function: print_init_status()

```c
static void print_init_status(const char *message, int status) {
    vga_putstr("[", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    if (status) {
        vga_putstr("OK", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr("FAIL", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    }
    vga_putstr("] ", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    vga_putstr(message, VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    vga_putstr("\n", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}
```

**Purpose**: Display colored status messages during boot.

**Usage**:
```c
print_init_status("Memory Manager", pmm_init(128 * 1024 * 1024));
/* Output: [OK] Memory Manager  (in green) */
```

### Error Handling

```c
if (!gdt_init()) {
    vga_putstr("FATAL: GDT initialization failed\n", 
               VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    goto halt;
}

halt:
    cli();
    hlt();
```

**Pattern**: If critical subsystem fails, halt system with error message.

---

## GDT Implementation (gdt.c)

**Location**: [src/kernel/arch/x86/gdt.c](src/kernel/arch/x86/gdt.c)

The Global Descriptor Table defines memory segments for protected mode.

### GDT Structure

```c
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t flags;          /* Flags and access rights */
    uint8_t limit_high;
    uint8_t base_high;
} gdt_entry_t;
```

**Size**: Exactly 8 bytes (as per x86 spec).

**Byte Layout**:
```
Bytes 0-1:   Limit (lower 16 bits)
Bytes 2-4:   Base address (24 bits)
Byte 5:      Access flags (Present, Privilege, Type)
Byte 6:      Flags and limit (upper 4 bits)
Byte 7:      Base address (upper 8 bits)
```

### Access Byte Structure

```c
#define GDT_ACCESS_PRESENT      0x80    /* Bit 7: Present */
#define GDT_ACCESS_RING3        0x60    /* Bits 6-5: Privilege level */
#define GDT_ACCESS_EXEC         0x08    /* Bit 3: Code segment */
#define GDT_ACCESS_RW           0x02    /* Bit 1: Readable/Writable */
```

**Example - Kernel Code**:
```
Access byte = 0x9A (10011010)
  Bit 7: 1 (Present)
  Bits 6-5: 00 (Ring 0)
  Bit 4: 1 (Descriptor)
  Bit 3: 1 (Code segment)
  Bit 2: 0 (Non-conforming)
  Bit 1: 1 (Readable)
  Bit 0: 0
```

### Flat Model Setup

```c
int gdt_init(void) {
    /* Null descriptor (always required) */
    gdt[0] = create_entry(0x00000000, 0x00000000, 0x00, 0x00);
    
    /* Kernel code segment at ring 0 */
    gdt[1] = create_entry(0x00000000, 0xFFFFFFFF, 
                         0x9A,  /* Present, Ring 0, Code, Exec */
                         0xCF); /* 4KB pages, 32-bit */
    
    /* Kernel data segment at ring 0 */
    gdt[2] = create_entry(0x00000000, 0xFFFFFFFF,
                         0x92,  /* Present, Ring 0, Data, RW */
                         0xCF);
    
    /* User code segment at ring 3 */
    gdt[3] = create_entry(0x00000000, 0xFFFFFFFF,
                         0xFA,  /* Present, Ring 3, Code */
                         0xCF);
    
    /* User data segment at ring 3 */
    gdt[4] = create_entry(0x00000000, 0xFFFFFFFF,
                         0xF2,  /* Present, Ring 3, Data */
                         0xCF);
    
    gdt_load();
    return 0;
}
```

**Flat Model**: All segments have:
- Base = 0x00000000
- Limit = 0xFFFFFFFF (entire 4 GB address space)

**Result**: Segments fully overlap, giving access to entire memory.

### GDT Load

```c
void gdt_load(void) {
    gdt_register_t gdtr;
    gdtr.size = (sizeof(gdt_entry_t) * 6) - 1;   /* Size in bytes - 1 */
    gdtr.offset = (uint32_t)gdt;                 /* Linear address */
    
    asm volatile("lgdt %0" : : "m"(gdtr));      /* Load GDT */
    
    asm volatile(
        "ljmp $0x08, $1f\n"  /* Far jump with new code selector (0x08) */
        "1:"
    );
}
```

**Purpose**: Load GDTR and reload code segment.

**Why far jump**: Necessary to reload CS (code segment) with new selector.

**Why 0x08**: Selector for kernel code (index 1, TI=0, RPL=0):
- Index bits: 1 << 3 = 0x08

---

## IDT & Interrupts (idt.c)

**Location**: [src/kernel/arch/x86/idt.c](src/kernel/arch/x86/idt.c)

Interrupt Descriptor Table maps interrupt vectors to handler routines.

### IDT Entry Structure

```c
typedef struct {
    uint16_t offset_low;            /* Lower 16 bits of handler address */
    uint16_t selector;              /* GDT selector (usually 0x08 for kernel) */
    uint8_t zero;                   /* Reserved, must be 0 */
    uint8_t flags;                  /* Type and DPL */
    uint16_t offset_high;           /* Upper 16 bits of handler address */
} idt_entry_t;
```

**Size**: Exactly 8 bytes.

### Flags Byte

```c
#define IDT_FLAG_PRESENT        0x80    /* Bit 7: Present */
#define IDT_FLAG_RING0          0x00    /* Bits 6-5: Ring 0 */
#define IDT_FLAG_RING3          0x60    /* Bits 6-5: Ring 3 */
#define IDT_FLAG_INTERRUPT      0x0E    /* Bits 3-0: Interrupt gate */
```

### Handler Address Encoding

```c
offset_low = (uint32_t)handler & 0x0000FFFF;
offset_high = ((uint32_t)handler & 0xFFFF0000) >> 16;
```

**Example**: If handler at 0x8001234:
- offset_low = 0x1234
- offset_high = 0x8001

### Setting Up Handlers

```c
void idt_set_gate(int vector, void (*handler)(void), int dpl) {
    idt_entry_t *entry = &idt[vector];
    
    uint32_t handler_addr = (uint32_t)handler;
    
    entry->offset_low = handler_addr & 0xFFFF;
    entry->selector = 0x08;                     /* Kernel code segment */
    entry->zero = 0;
    entry->flags = 0x80 | (dpl << 5) | 0x0E;   /* Present, DPL, Interrupt */
    entry->offset_high = (handler_addr >> 16) & 0xFFFF;
}
```

### IDT Initialization

```c
int idt_init(void) {
    /* Clear all entries */
    memset(idt, 0, sizeof(idt));
    
    /* Set exception handlers (0-31) */
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (void *)exception_handlers[i], 0);
    }
    
    /* Set IRQ handlers (32-47) */
    for (int i = 0; i < 16; i++) {
        idt_set_gate(32 + i, (void *)irq_handlers[i], 0);
    }
    
    /* Set syscall handler (vector 128) */
    idt_set_gate(128, (void *)syscall_handler, 3);  /* User accessible */
    
    idt_load();
    return 0;
}
```

### Handler Example: Timer (IRQ0)

```asm
irq0_handler:
    PUSH EAX
    PUSH ECX
    PUSH EDX
    PUSH EDI
    PUSH ESI
    PUSH EBP
    
    CALL scheduler_tick         ; Call C function
    
    MOV AL, 0x20
    OUT 0x20, AL                ; Send EOI (End of Interrupt) to PIC
    
    POP EBP
    POP ESI
    POP EDI
    POP EDX
    POP ECX
    POP EAX
    IRET                        ; Return from interrupt
```

**Critical steps**:
1. Save all registers
2. Call C handler (scheduler_tick)
3. Send EOI to Programmable Interrupt Controller
4. Restore registers
5. IRET (restores EIP and EFLAGS)

---

## Physical Memory Manager (pmm.c)

**Location**: [src/kernel/mm/pmm.c](src/kernel/mm/pmm.c)

Manages physical RAM using bitmap allocator.

### Bitmap Approach

```c
typedef struct {
    uint32_t total_pages;       /* Total pages (128 MB = 32K pages) */
    uint32_t free_pages;        /* Currently free */
    uint8_t *bitmap;            /* 1 bit per page: 0=free, 1=allocated */
} pmm_t;

/* Global PMM state */
static pmm_t pmm;
```

**Bitmap size**: 32KB pages / 8 bits per byte = 4 KB

**Memory layout**:
```
0x00001000  ┌─────────────────────┐
            │  PMM Bitmap (4 KB)  │
            ├─────────────────────┤
0x00002000  │  Heap grows here    │
            │                     │
            ├─────────────────────┤
0x00100000  │  Kernel at 1 MB     │
            │  (linked here)      │
            └─────────────────────┘
```

### Initialization

```c
int pmm_init(uint32_t total_memory) {
    pmm.total_pages = total_memory / PMM_PAGE_SIZE;  /* Divide by 4096 */
    pmm.free_pages = pmm.total_pages;
    
    /* Allocate bitmap at 0x1000 */
    pmm.bitmap = (uint8_t *)0x1000;
    
    /* Mark all pages as free */
    memset(pmm.bitmap, 0, (pmm.total_pages + 7) / 8);
    
    /* Mark reserved pages (bootloader, kernel, bitmap itself) */
    for (uint32_t i = 0; i < 0x100000; i += 0x1000) {
        pmm_mark_used(i);  /* Mark first MB as used */
    }
    
    return 0;
}
```

### Allocation Algorithm

```c
uint32_t pmm_alloc(void) {
    /* Find first free bit in bitmap */
    for (uint32_t i = 0; i < pmm.total_pages; i++) {
        uint32_t byte_idx = i / 8;
        uint32_t bit_idx = i % 8;
        
        if (!(pmm.bitmap[byte_idx] & (1 << bit_idx))) {
            /* Found free page */
            pmm.bitmap[byte_idx] |= (1 << bit_idx);  /* Mark as used */
            pmm.free_pages--;
            
            return (phys_addr_t)(i * PMM_PAGE_SIZE);
        }
    }
    
    return 0xFFFFFFFF;  /* No free pages */
}
```

**Complexity**: O(n) where n = number of pages

**Optimization opportunity**: Use optimized bit searching (clz instruction).

### Deallocation

```c
void pmm_free(uint32_t addr) {
    uint32_t page_num = addr / PMM_PAGE_SIZE;
    uint32_t byte_idx = page_num / 8;
    uint32_t bit_idx = page_num % 8;
    
    pmm.bitmap[byte_idx] &= ~(1 << bit_idx);  /* Mark as free */
    pmm.free_pages++;
}
```

---

## Virtual Memory Manager (vmm.c)

**Location**: [src/kernel/mm/vmm.c](src/kernel/mm/vmm.c)

Implements page-based virtual memory with paging.

### Paging Data Structures

```c
/* Page table entry format (32-bit) */
#define PTE_PRESENT         0x001
#define PTE_WRITABLE        0x002
#define PTE_USER            0x004
#define PTE_WRITE_THROUGH   0x008
#define PTE_CACHE_DISABLE   0x010
#define PTE_ACCESSED        0x020
#define PTE_DIRTY           0x040
#define PTE_PAT             0x080
#define PTE_GLOBAL          0x100

/* Address extraction from virtual address */
#define PD_INDEX(va)    ((va) >> 22)        /* Bits 31-22 */
#define PT_INDEX(va)    (((va) >> 12) & 0x3FF)  /* Bits 21-12 */
#define PAGE_OFFSET(va) ((va) & 0xFFF)     /* Bits 11-0 */

typedef uint32_t pde_t;     /* Page directory entry */
typedef uint32_t pte_t;     /* Page table entry */

static pde_t kernel_page_dir[1024];    /* One page directory for kernel */
```

### Virtual Address Translation

```
Virtual Address (32-bit):
┌─────────────────┬──────────────┬─────────────┐
│  Dir (10 bits)  │ Table (10)   │ Offset (12) │
└──────┬──────────┴──────┬───────┴──────┬──────┘
       │                 │              │
       ▼                 ▼              ▼
   Page Dir       Page Table        Physical Page
   [1024 PDE]     [1024 PTE]         (4 KB)
       │               │
       └───────────────┴── → Physical Address
```

### Initialization

```c
int vmm_init(void) {
    /* Create kernel page directory */
    memset(kernel_page_dir, 0, 4096);
    
    /* Identity map first 4 MB (for bootloader safety) */
    for (uint32_t i = 0; i < 0x400000; i += 0x1000) {
        vmm_map_page(i, i, PTE_PRESENT | PTE_WRITABLE);
    }
    
    /* Create kernel's higher-half mapping (0xC0000000+) */
    for (uint32_t i = 0; i < 0x400000; i += 0x1000) {
        vmm_map_page(0xC0000000 + i, i, 
                    PTE_PRESENT | PTE_WRITABLE | PTE_GLOBAL);
    }
    
    /* Enable paging by setting CR0 bit 31 */
    uint32_t cr0;
    asm("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm("mov %0, %%cr0" : : "r"(cr0));
    
    /* Load page directory into CR3 */
    vmm_set_page_dir((uint32_t)kernel_page_dir);
    
    return 0;
}
```

### Page Mapping

```c
int vmm_map_page(uint32_t virt, uint32_t phys, int flags) {
    uint32_t pd_idx = PD_INDEX(virt);
    uint32_t pt_idx = PT_INDEX(virt);
    
    /* Get or create page table */
    pte_t *page_table;
    if (!(kernel_page_dir[pd_idx] & PTE_PRESENT)) {
        /* Allocate new page table */
        uint32_t pt_phys = pmm_alloc();
        if (pt_phys == 0xFFFFFFFF) return -1;
        
        memset((void *)pt_phys, 0, 4096);  /* Clear page table */
        kernel_page_dir[pd_idx] = pt_phys | PTE_PRESENT | PTE_WRITABLE;
        
        page_table = (pte_t *)pt_phys;
    } else {
        page_table = (pte_t *)(kernel_page_dir[pd_idx] & 0xFFFFF000);
    }
    
    /* Set page table entry */
    page_table[pt_idx] = (phys & 0xFFFFF000) | flags;
    
    /* Invalidate TLB entry */
    asm("invlpg %0" : : "m"(*(char *)virt));
    
    return 0;
}
```

**Key operation: INVLPG**
- Invalidates TLB entry for that page
- Ensures CPU reloads mapping from page tables

### Address Translation

```c
uint32_t vmm_get_physical(uint32_t virt) {
    uint32_t pd_idx = PD_INDEX(virt);
    uint32_t pt_idx = PT_INDEX(virt);
    
    if (!(kernel_page_dir[pd_idx] & PTE_PRESENT)) {
        return 0xFFFFFFFF;  /* Page table not present */
    }
    
    pte_t *page_table = (pte_t *)(kernel_page_dir[pd_idx] & 0xFFFFF000);
    
    if (!(page_table[pt_idx] & PTE_PRESENT)) {
        return 0xFFFFFFFF;  /* Page not mapped */
    }
    
    uint32_t phys = page_table[pt_idx] & 0xFFFFF000;
    return phys + PAGE_OFFSET(virt);
}
```

---

## Process Management (process.c)

**Location**: [src/kernel/proc/process.c](src/kernel/proc/process.c)

Manages process creation, state, and lifecycle.

### Process Control Block

```c
typedef struct {
    uint32_t pid;                       /* Process identifier */
    uint32_t parent_pid;
    uint32_t state;                     /* READY, RUNNING, BLOCKED, ZOMBIE */
    uint32_t priority;
    
    interrupt_context_t *context;       /* CPU state at context switch */
    
    uint32_t page_dir_phys;             /* Virtual memory root */
    uint32_t kernel_stack;              /* Kernel stack base */
    uint32_t user_stack;                /* User stack base (unused in this OS) */
    
    uint32_t time_slice_remaining;
    uint32_t total_runtime;
    uint32_t wait_queue;
    
    void *blocked_on;
} pcb_t;

static pcb_t process_table[MAX_PROCESSES];
static uint32_t next_pid = 1;
static pcb_t *current_process = NULL;
```

### Process Creation

```c
uint32_t proc_create(void (*entry)(void), int priority) {
    /* Find free PCB slot */
    uint32_t pid = 0;
    pcb_t *new_pcb = NULL;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == 0) {
            new_pcb = &process_table[i];
            pid = next_pid++;
            break;
        }
    }
    
    if (!new_pcb) return 0xFFFFFFFF;  /* No free processes */
    
    /* Create page directory for process */
    uint32_t page_dir = vmm_create_page_dir();
    if (page_dir == 0xFFFFFFFF) return 0xFFFFFFFF;
    
    /* Allocate kernel stack */
    uint32_t stack_phys = pmm_alloc();
    if (stack_phys == 0xFFFFFFFF) return 0xFFFFFFFF;
    
    /* Initialize PCB */
    new_pcb->pid = pid;
    new_pcb->parent_pid = (current_process) ? current_process->pid : 0;
    new_pcb->state = PROC_STATE_READY;
    new_pcb->priority = priority;
    new_pcb->page_dir_phys = page_dir;
    new_pcb->kernel_stack = stack_phys + 4096;  /* Top of stack */
    new_pcb->user_stack = 0x10000;  /* Stub */
    new_pcb->time_slice_remaining = SCHEDULER_TIME_SLICE;
    new_pcb->total_runtime = 0;
    
    /* Set up entry point in context */
    interrupt_context_t *ctx = (interrupt_context_t *)
                                (new_pcb->kernel_stack - sizeof(*ctx));
    memset(ctx, 0, sizeof(*ctx));
    ctx->eip = (uint32_t)entry;
    ctx->eflags = 0x200;  /* IF flag (interrupts enabled) */
    ctx->cs = 0x08;  /* Kernel code */
    ctx->ds = 0x10;  /* Kernel data */
    ctx->es = 0x10;
    ctx->fs = 0x10;
    ctx->gs = 0x10;
    ctx->ss = 0x10;
    
    new_pcb->context = ctx;
    
    return pid;
}
```

**Key design**:
- Pre-initialize registers so context switch works immediately
- Set EIP to entry point
- Set flags to enable interrupts
- Load context from stack pointer

### Process Exit

```c
void proc_exit(int exit_code) {
    if (!current_process) return;
    
    /* Mark as zombie */
    current_process->state = PROC_STATE_ZOMBIE;
    
    /* Free resources (simplified - no cleanup of memory) */
    
    /* Switch to next process */
    schedule();
    
    /* Should never reach here */
}
```

---

## Scheduler (scheduler.c)

**Location**: [src/kernel/proc/scheduler.c](src/kernel/proc/scheduler.c)

Implements round-robin process scheduling.

### Scheduler State

```c
typedef struct {
    pcb_t *ready_queue[MAX_PROCESSES];
    uint32_t queue_head;
    uint32_t queue_tail;
} scheduler_state_t;

static scheduler_state_t scheduler;
static uint32_t ticks = 0;  /* Global tick counter */
```

### Timer Initialization

```c
int scheduler_init(void) {
    memset(&scheduler, 0, sizeof(scheduler_state_t));
    
    /* Initialize PIT (Programmable Interval Timer) */
    uint32_t divisor = 1193180 / 100;  /* 100 Hz = 10 ms */
    
    /* Send command byte */
    outb(0x43, 0x36);  /* 0x36 = 00110110 (counter 0, mode 3, 16-bit) */
    
    /* Send divisor LSB then MSB */
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
    
    return 0;
}
```

**PIT Details**:
- Base frequency: 1,193,180 Hz
- To get 100 Hz: divisor = 1,193,180 / 100 = 11,931 (0x2E9B)
- Every 10 ms, PIT fires IRQ0

### Scheduler Tick

```c
void scheduler_tick(void) {
    ticks++;
    
    if (current_process) {
        current_process->total_runtime++;
        current_process->time_slice_remaining--;
        
        /* If time slice exhausted, reschedule */
        if (current_process->time_slice_remaining == 0) {
            schedule();
        }
    }
}
```

### Round-Robin Selection

```c
void schedule(void) {
    if (!current_process) {
        /* First schedule - find any READY process */
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].state == PROC_STATE_READY) {
                current_process = &process_table[i];
                current_process->state = PROC_STATE_RUNNING;
                current_process->time_slice_remaining = SCHEDULER_TIME_SLICE;
                break;
            }
        }
        return;
    }
    
    /* Save current process state */
    if (current_process->state == PROC_STATE_RUNNING) {
        current_process->state = PROC_STATE_READY;
    }
    
    /* Find next READY process after current */
    pcb_t *next = NULL;
    for (int i = (current_process->pid) % MAX_PROCESSES + 1; 
         i < current_process->pid + MAX_PROCESSES; i++) {
        pcb_t *p = &process_table[i % MAX_PROCESSES];
        if (p->pid != 0 && p->state == PROC_STATE_READY) {
            next = p;
            break;
        }
    }
    
    /* If no process found, use current (may not be READY) */
    if (!next) next = current_process;
    
    /* Switch context */
    switch_context(current_process, next);
    
    /* Update current */
    current_process = next;
    current_process->state = PROC_STATE_RUNNING;
    current_process->time_slice_remaining = SCHEDULER_TIME_SLICE;
}
```

**Algorithm**: Find next process in round-robin order.

**Fair scheduling**: Each process gets equal time slice (10 ticks = 100 ms).

---

## VGA Driver (vga.c)

**Location**: [src/kernel/drivers/vga.c](src/kernel/drivers/vga.c)

Text-mode display output through memory-mapped VGA buffer.

### VGA Memory Layout

```c
#define VGA_MEMORY      0xB8000
#define VGA_WIDTH       80
#define VGA_HEIGHT      25

typedef struct {
    char ch;        /* ASCII character */
    uint8_t attr;   /* Color attribute */
} vga_char_t;

static vga_char_t *vga_buf = (vga_char_t *)VGA_MEMORY;
static uint8_t cursor_x = 0, cursor_y = 0;
static uint8_t current_color = 0x0F;  /* White on black */
```

**Memory organization**:
```
0xB8000  ┌──────────────────┐
         │ Row 0 (80 chars) │  2 bytes per char
         │ Row 1 (80 chars) │
         │ ...              │
0xB8FA0  │ Row 24 (80 chars)│
         └──────────────────┘
```

### Putchar Implementation

```c
void vga_putchar(char ch, uint8_t color) {
    uint32_t offset = cursor_y * VGA_WIDTH + cursor_x;
    
    if (ch == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (ch == '\t') {
        cursor_x += 8;  /* Tab = 8 spaces */
    } else if (ch >= 32) {  /* Printable character */
        vga_buf[offset].ch = ch;
        vga_buf[offset].attr = color;
        cursor_x++;
    }
    
    /* Wrap to next line */
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    /* Scroll if needed */
    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll_up();
        cursor_y = VGA_HEIGHT - 1;
    }
    
    /* Update hardware cursor */
    vga_update_cursor();
}
```

### Scroll Implementation

```c
void vga_scroll_up(void) {
    /* Copy lines 1-24 to 0-23 */
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            uint32_t from = (y + 1) * VGA_WIDTH + x;
            uint32_t to = y * VGA_WIDTH + x;
            vga_buf[to] = vga_buf[from];
        }
    }
    
    /* Clear last line */
    for (int x = 0; x < VGA_WIDTH; x++) {
        uint32_t pos = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        vga_buf[pos].ch = ' ';
        vga_buf[pos].attr = current_color;
    }
}
```

### Cursor Control (I/O Ports)

```c
void vga_update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    
    /* Send cursor high byte */
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    
    /* Send cursor low byte */
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}
```

**I/O Ports**:
- 0x3D4: Address register (selects which register to access)
- 0x3D5: Data register (read/write data)

**Protocol**:
1. Write address (0x0E for cursor high)
2. Write data
3. Repeat for other fields

---

## Keyboard Driver (kbd.c)

**Location**: [src/kernel/drivers/kbd.c](src/kernel/drivers/kbd.c)

PS/2 keyboard input with scancode translation.

### Keyboard Data Structures

```c
#define KBD_BUFFER_SIZE  256

typedef struct {
    char ascii;         /* ASCII character */
    uint8_t scancode;   /* Raw scancode */
    int pressed;        /* 1=pressed, 0=released */
} key_event_t;

typedef struct {
    key_event_t buffer[KBD_BUFFER_SIZE];
    uint16_t head;      /* Write position */
    uint16_t tail;      /* Read position */
} kbd_buffer_t;

static kbd_buffer_t kbd_buf;
```

### Scancode to ASCII Mapping

```c
static const uint8_t scancode_map[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8',  /* 0x00-0x09 */
    '9', '0', '-', '=', 8, 9, 'q', 'w', 'e', 'r',   /* 0x0A-0x13 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13, 0,  /* 0x14-0x1D */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 0x1E-0x27 */
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 0x28-0x31 */
    'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0        /* 0x32-0x3B */
};
```

### IRQ1 Handler

```asm
irq1_handler:
    PUSHA
    
    IN AL, 0x60         ; Read scancode from keyboard
    
    ; Check if key pressed (high bit = 0) or released (high bit = 1)
    TEST AL, 0x80
    JNZ key_released
    
    ; Translate scancode to ASCII
    MOVZX EAX, AL
    LEA EBX, [scancode_map]
    MOV AL, [EBX + EAX]
    
    ; Add to buffer
    CALL kbd_buffer_push
    
key_released:
    MOV AL, 0x20
    OUT 0x20, AL        ; Send EOI to PIC
    
    POPA
    IRET
```

**Scancode meanings**:
- Bits 7-0: Actual scancode
- Bit 7: 0=pressed, 1=released

### Buffer Management

```c
void kbd_buffer_push(key_event_t event) {
    uint16_t next_head = (kbd_buf.head + 1) % KBD_BUFFER_SIZE;
    
    /* Check if buffer full */
    if (next_head == kbd_buf.tail) {
        return;  /* Buffer overflow - drop character */
    }
    
    kbd_buf.buffer[kbd_buf.head] = event;
    kbd_buf.head = next_head;
}

int kbd_get_key(void) {
    if (kbd_buf.head == kbd_buf.tail) {
        return -1;  /* No data */
    }
    
    key_event_t event = kbd_buf.buffer[kbd_buf.tail];
    kbd_buf.tail = (kbd_buf.tail + 1) % KBD_BUFFER_SIZE;
    
    return event.ascii;
}
```

---

## Shell Implementation (shell.c)

**Location**: [src/kernel/shell/shell.c](src/kernel/shell/shell.c)

Command-line user interface for interacting with OS.

### Shell State

```c
#define SHELL_BUFFER_SIZE   256
#define SHELL_MAX_ARGS      32

typedef struct {
    char line[SHELL_BUFFER_SIZE];   /* Current input line */
    int32_t pos;                    /* Cursor position */
    char *argv[SHELL_MAX_ARGS];     /* Parsed arguments */
    int32_t argc;                   /* Argument count */
} shell_state_t;

static shell_state_t shell;
```

### Command Table

```c
typedef void (*cmd_handler_t)(int argc, char **argv);

typedef struct {
    const char *name;
    cmd_handler_t handler;
} command_t;

static const command_t commands[] = {
    {"help",    cmd_help},
    {"echo",    cmd_echo},
    {"clear",   cmd_clear},
    {"mem",     cmd_mem},
    {"ps",      cmd_ps},
    {"uname",   cmd_uname},
    {"reboot",  cmd_reboot},
    {"info",    cmd_info},
    {NULL, NULL}
};
```

### Main Shell Loop

```c
int shell_run(void) {
    while (1) {
        shell_print_prompt();
        
        /* Read input line */
        int pos = 0;
        while (1) {
            int ch = kbd_get_key();
            
            if (ch == -1) {
                /* No key, yield to scheduler */
                yield();
                continue;
            }
            
            if (ch == '\n' || ch == '\r') {
                shell.line[pos] = '\0';
                vga_putstr("\n", VGA_COLOR_WHITE);
                break;
            } else if (ch == '\b' && pos > 0) {
                /* Backspace */
                vga_putchar('\b', VGA_COLOR_WHITE);
                pos--;
            } else if (ch >= 32 && pos < SHELL_BUFFER_SIZE - 1) {
                /* Regular character */
                shell.line[pos] = ch;
                vga_putchar(ch, VGA_COLOR_WHITE);
                pos++;
            }
        }
        
        /* Parse and execute */
        parse_line(shell.line);
        execute_command();
    }
    
    return 0;
}
```

### Command Parsing

```c
static void parse_line(const char *line) {
    shell.argc = 0;
    char *copy = (char *)kmalloc(strlen(line) + 1);
    strcpy(copy, line);
    
    char *token = strtok(copy, " \t");
    while (token && shell.argc < SHELL_MAX_ARGS) {
        shell.argv[shell.argc++] = token;
        token = strtok(NULL, " \t");
    }
}

static void execute_command(void) {
    if (shell.argc == 0) return;
    
    const char *cmd_name = shell.argv[0];
    
    for (int i = 0; commands[i].name; i++) {
        if (strcmp(cmd_name, commands[i].name) == 0) {
            commands[i].handler(shell.argc, shell.argv);
            return;
        }
    }
    
    vga_putstr("Unknown command: ", VGA_COLOR_RED);
    vga_putstr(cmd_name, VGA_COLOR_RED);
    vga_putstr("\n", VGA_COLOR_RED);
}
```

### Example Command: Echo

```c
static void cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        vga_putstr(argv[i], VGA_COLOR_YELLOW);
        if (i < argc - 1) {
            vga_putstr(" ", VGA_COLOR_YELLOW);
        }
    }
    vga_putstr("\n", VGA_COLOR_YELLOW);
}
```

### Example Command: Memory Stats

```c
static void cmd_mem(int argc, char **argv) {
    uint32_t total = pmm_get_total_pages() * 4096;
    uint32_t free = pmm_get_free_pages() * 4096;
    uint32_t used = total - free;
    
    vga_putstr("Memory Statistics:\n", VGA_COLOR_GREEN);
    vga_putstr("─────────────────────────────────\n", VGA_COLOR_GREEN);
    
    /* Print formatted output */
    char buf[128];
    sprintf(buf, "Total: %d KB\n", total / 1024);
    vga_putstr(buf, VGA_COLOR_CYAN);
    
    sprintf(buf, "Free:  %d KB\n", free / 1024);
    vga_putstr(buf, VGA_COLOR_GREEN);
    
    sprintf(buf, "Used:  %d KB\n", used / 1024);
    vga_putstr(buf, VGA_COLOR_YELLOW);
}
```

---

## Summary

**Key Implementation Patterns**:

1. **Bootloader** → Kernel → Subsystems → Shell
2. **Memory**: PMM (bitmap) → VMM (paging) → Heap (first-fit)
3. **Interrupts**: GDT (segments) → IDT (handlers) → IRQs (devices)
4. **Processes**: PCB tables → Round-robin scheduler → Timer interrupts
5. **Drivers**: Memory-mapped I/O or port I/O → Interrupt handlers
6. **Shell**: Polling keyboard → Parsing → Command table lookup → Execute

**Critical Design Decisions**:
- Higher-half kernel for cleaner virtual memory
- Preemptive scheduling with fixed time slices (fair scheduling)
- Bitmap PMM (simple, O(n) but works for 128 MB)
- Two-level page tables (manageable complexity)
- Ring 0 kernel (no user mode protection)

---

**Implementation Guide Version**: 1.0  
**Last Updated**: May 30, 2026

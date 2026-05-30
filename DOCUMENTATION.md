# NIMBU OS - Complete Technical Documentation

**A Minimal Linux-Based Operating System**

**Version**: 1.0 | **Architecture**: x86 32-bit | **Status**: Complete

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [System Architecture](#system-architecture)
3. [Memory Management](#memory-management)
4. [Interrupt Handling](#interrupt-handling)
5. [Process Management](#process-management)
6. [Device Drivers](#device-drivers)
7. [Shell Interface](#shell-interface)
8. [Build System](#build-system)
9. [API Reference](#api-reference)
10. [Implementation Guide](#implementation-guide)
11. [Configuration & Customization](#configuration--customization)
12. [Troubleshooting](#troubleshooting)

---

## Project Overview

### What is NIMBU OS?

NIMBU OS is a minimalist, educational operating system kernel written from scratch for x86 32-bit architecture. It demonstrates fundamental OS concepts including:

- **Kernel Architecture**: Monolithic kernel design
- **Memory Management**: Physical memory manager (PMM), virtual memory with paging, and dynamic heap allocation
- **Process Management**: Multi-process support with round-robin preemptive scheduler
- **Interrupt Handling**: IDT-based exception and interrupt dispatch
- **Device Drivers**: VGA display, PS/2 keyboard, and serial port support
- **User Interface**: Command-line shell with basic utilities

### Key Characteristics

| Aspect | Details |
|--------|---------|
| **CPU Architecture** | x86 (Intel i686) 32-bit |
| **Memory Model** | Higher-half kernel (0xC0000000+), 128 MB RAM support |
| **Task Model** | Preemptive multitasking with round-robin scheduling |
| **Execution Mode** | Protected mode (Ring 0 kernel, Ring 3 user) |
| **Interrupt Frequency** | 100 Hz (10 ms ticks) |
| **Max Processes** | 256 concurrent processes |
| **Kernel Stack** | 8 KB per process |

### Project Goals

1. **Educational Value**: Demonstrate how OS fundamentals work in practice
2. **Minimal Complexity**: Include only essential components, no unnecessary bloat
3. **Clear Code**: Well-documented, readable implementation
4. **Complete Pipeline**: From bootloader through shell with proper toolchain setup

---

## System Architecture

### Layered Architecture Diagram

```
┌─────────────────────────────────┐
│  Layer 5: User Applications     │  Shell, utilities
├─────────────────────────────────┤
│  Layer 4: Device Drivers        │  VGA, Keyboard, Serial, Timer
├─────────────────────────────────┤
│  Layer 3: Interrupt Handling    │  IDT, ISR, IRQ handlers
├─────────────────────────────────┤
│  Layer 2: Process Management    │  Scheduler, PCBs, Context Switch
├─────────────────────────────────┤
│  Layer 1: Memory Management     │  PMM, VMM, Heap, Paging
├─────────────────────────────────┤
│  Layer 0: CPU & Bootloader      │  GDT, Protected Mode, Boot
└─────────────────────────────────┘
```

### Boot Sequence

```
1. BIOS/Bootloader (_start in boot.asm)
   ├─ Multiboot header verification
   ├─ Stack initialization (8 KB)
   ├─ Clear EFLAGS register
   └─ Call kmain()

2. Kernel Initialization (kmain in kernel.c)
   ├─ VGA driver init (for console output)
   ├─ GDT initialization (segment descriptors)
   ├─ IDT initialization (interrupt handlers)
   ├─ Memory management init (PMM, VMM, Heap)
   ├─ Interrupt handler setup
   ├─ Keyboard driver init
   ├─ Process management init
   ├─ Timer initialization
   └─ Launch shell

3. Shell Execution
   ├─ Display prompt
   ├─ Read keyboard input
   ├─ Parse and execute commands
   └─ Handle scheduling/interrupts
```

### Memory Layout (32-bit x86)

```
Virtual Address Space (4 GB total)

0xFFFFFFFF ┌──────────────────────┐
           │  Kernel Base (1 GB)  │
0xC0400000 ├──────────────────────┤
           │  Kernel Heap (8 MB)  │
0xC0100000 ├──────────────────────┤
           │  Kernel Code/Data    │  Higher-half kernel
           │  (linked at 0xC0..)  │
0xC0000000 ├──────────────────────┤
           │                      │
           │                      │
           │  User Space (3 GB)   │  Not fully utilized
           │  (available for)     │
           │  user processes)     │
           │                      │
0x00100000 ├──────────────────────┤
           │  Kernel Load Addr    │  Physical: 0x100000
           │  (identity mapped)   │
0x00000000 └──────────────────────┘
           Real Mode IVT (1st 1KB)
```

### Segment Layout

```
GDT Entry   Descriptor Type     Base        Limit      Ring
─────────   ──────────────────  ──────────  ────────   ────
0x00        Null (unused)       0x00000000  0x00000    N/A
0x08        Kernel Code         0x00000000  0xFFFFFF   0
0x10        Kernel Data         0x00000000  0xFFFFFF   0
0x18        User Code           0x00000000  0xFFFFFF   3
0x20        User Data           0x00000000  0xFFFFFF   3
(rest)      TSS, LDT (unused)   various     various    N/A
```

---

## Memory Management

NIMBU OS uses a three-tier memory management approach:

### 1. Physical Memory Manager (PMM)

**File**: [src/kernel/mm/pmm.c](src/kernel/mm/pmm.c)

The PMM manages physical RAM using a bitmap allocator.

#### Key Features:
- **Allocation Unit**: 4 KB pages
- **Tracking**: Bitmap with 1 bit per page (1 bit covers 4 KB)
- **Max Memory**: 128 MB (32K pages)
- **Interface**: `pmm_alloc()`, `pmm_free()`, `pmm_init()`

#### Algorithm:
```
pmm_alloc():
1. Scan bitmap for free page (bit = 0)
2. Mark as allocated (bit = 1)
3. Return physical address

pmm_free(addr):
1. Calculate bitmap index
2. Mark bit as 0
3. Page is available for reallocation
```

#### Data Structures:
```c
typedef struct {
    uint32_t total_pages;        /* Total pages available */
    uint32_t free_pages;         /* Currently free pages */
    uint8_t *bitmap;             /* Bitmap (1 bit per page) */
} pmm_state_t;
```

### 2. Virtual Memory Manager (VMM)

**File**: [src/kernel/mm/vmm.c](src/kernel/mm/vmm.c)

The VMM manages virtual-to-physical address mapping using x86 paging.

#### Paging Structure:
```
Virtual Address (32-bit):
┌─────────────────┬──────────────┬─────────────┐
│ Dir Index (10)  │ Table Index  │ Offset (12) │
│  bits 31-22     │  bits 21-12  │  bits 11-0  │
└─────────────────┴──────────────┴─────────────┘
         ↓               ↓              ↓
    Page Directory   Page Table    Physical Page
      (1024 entries) (1024 entries)  (4 KB)
```

#### Key Features:
- **Page Size**: 4 KB
- **Directory Entries**: 1024 per directory
- **Table Entries**: 1024 per table
- **Page Attribute Bits**: Present, Writable, User/Supervisor, Write-Through, Cache-Disable, Accessed, Dirty, PSE, Global
- **Higher-Half Kernel**: Kernel at 0xC0000000+ with identity mapping

#### Functions:
```c
vmm_init()              /* Initialize paging */
vmm_create_page_dir()   /* Allocate new page directory */
vmm_map_page()          /* Map virtual → physical */
vmm_unmap_page()        /* Unmap page */
vmm_get_physical()      /* Translate virtual → physical */
vmm_set_page_dir()      /* Install CR3 (page dir base) */
```

### 3. Kernel Heap

**File**: [src/kernel/mm/heap.c](src/kernel/mm/heap.c)

The heap provides dynamic memory allocation using first-fit algorithm.

#### Configuration:
```c
#define HEAP_BASE       0xC0100000  /* Start address */
#define HEAP_MAX_SIZE   0x800000    /* 8 MB maximum */
```

#### Metadata:
```c
typedef struct heap_block {
    uint32_t size;              /* Block size in bytes */
    struct heap_block *next;    /* Next block pointer */
    int32_t free;               /* 1 if free, 0 if allocated */
} heap_block_t;
```

#### Algorithm:
```
kmalloc(size):
1. Find first free block ≥ size
2. Mark as allocated
3. Split if larger than needed
4. Return pointer to data

kfree(ptr):
1. Find metadata (stored before data)
2. Mark as free
3. Merge with adjacent free blocks
```

#### Interface:
```c
void *kmalloc(size_t size);        /* Allocate memory */
void kfree(void *ptr);             /* Free memory */
void *krealloc(void *ptr, size_t size);  /* Resize allocation */
```

---

## Interrupt Handling

NIMBU OS uses the Interrupt Descriptor Table (IDT) for managing CPU exceptions and external interrupts.

### Interrupt Vector Layout

```
Vector Range    Type              Count   Priority
─────────────   ────────────────  ─────   ────────
0-31            CPU Exceptions    32      Highest
32-47           Hardware IRQ      16      High
48-127          Reserved/Available 80     (unused)
128 (0x80)      Syscall           1       Medium
129+            Available         127     (unused)
```

### Architecture Component Files

#### [src/kernel/arch/x86/gdt.c](src/kernel/arch/x86/gdt.c) - Global Descriptor Table

The GDT defines memory segments for protected mode operation.

**Functions**:
```c
gdt_init()                          /* Initialize GDT */
gdt_set_gate(index, base, limit)    /* Add descriptor entry */
gdt_load()                          /* Load GDT into GDTR */
```

**Structure**:
```c
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t flags;
    uint8_t limit_high;
    uint8_t base_high;
} gdt_entry_t;

typedef struct {
    uint16_t size;
    uint32_t offset;
} gdt_register_t;
```

#### [src/kernel/arch/x86/idt.c](src/kernel/arch/x86/idt.c) - Interrupt Descriptor Table

The IDT maps interrupt vectors to handler routines.

**Functions**:
```c
idt_init()                          /* Initialize IDT */
idt_set_gate(vector, handler, dpl)  /* Install handler */
idt_load()                          /* Load IDT into IDTR */
```

**Exception Handlers**:
```c
isr_0()   - Divide by Zero
isr_1()   - Debug
isr_2()   - NMI Interrupt
...
isr_31()  - Reserved
```

**IRQ Handlers**:
```c
irq_0()   - Timer (PIT)
irq_1()   - Keyboard
irq_2-14  - Available
irq_15()  - Secondary PIC
```

**Context Structure**:
```c
typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp, esp;
    uint32_t eip, eflags;
    uint32_t cs, ss, ds, es, fs, gs;
    uint32_t error_code;
} interrupt_context_t;
```

---

## Process Management

NIMBU OS supports up to 256 concurrent processes with preemptive round-robin scheduling.

### Process Control Block (PCB)

**File**: [include/process.h](include/process.h)

```c
typedef struct {
    uint32_t pid;                       /* Unique process ID */
    uint32_t parent_pid;                /* Parent process ID */
    uint32_t state;                     /* READY, RUNNING, BLOCKED, ZOMBIE */
    uint32_t priority;                  /* Priority level */
    
    interrupt_context_t *context;       /* Saved CPU registers */
    
    uint32_t page_dir_phys;             /* Physical addr of page directory */
    uint32_t kernel_stack;              /* Kernel stack pointer */
    uint32_t user_stack;                /* User stack pointer */
    
    uint32_t time_slice_remaining;      /* Ticks left in current timeslice */
    uint32_t total_runtime;             /* Total CPU time used */
    uint32_t wait_queue;                /* Queue if blocked */
    
    void *blocked_on;                   /* Resource being waited for */
} pcb_t;
```

### Process States

```
        ┌─────────────┐
        │   RUNNING   │  Currently executing
        └──────┬──────┘
               │ timer tick / yield
        ┌──────▼──────────────┐
        │   READY             │  Ready to run, waiting for CPU
        └──────┬──────────────┘
               │ switch_to()
        ┌──────▼──────────────┐
        │   BLOCKED           │  Waiting for I/O or resource
        ├─────────────────────┤
        │  - Wait queue       │
        │  - blocked_on field │
        └─────────────────────┘
               ▲
               │ I/O complete
               │
```

### Scheduler

**File**: [src/kernel/proc/scheduler.c](src/kernel/proc/scheduler.c)

The scheduler implements round-robin preemptive multitasking.

**Key Parameters**:
```c
#define SCHEDULER_TIME_SLICE    10      /* 100 ms at 100 Hz timer */
#define MAX_PROCESSES           256
```

**Algorithm**:
```
On each timer tick (IRQ0):
1. Call scheduler_tick()
2. Decrement current process time_slice_remaining
3. If time_slice == 0:
   a. Save current process context
   b. Mark as READY
   c. Find next READY process
   d. Load its context
   e. Mark as RUNNING
   f. Update time_slice_remaining
4. Return from interrupt (resume process)
```

**Core Functions**:
```c
proc_init()                         /* Initialize process management */
proc_create(entry, priority)        /* Create new process */
proc_exit(exit_code)                /* Terminate process */
scheduler_init()                    /* Initialize scheduler */
schedule()                          /* Find next process to run */
switch_context(from, to)            /* Context switch */
```

### Context Switching

Context switching occurs at:
1. **Timer Interrupt**: When time slice expires (10 ticks = 100 ms)
2. **Voluntary Yield**: Process calls `yield()`
3. **I/O Block**: Process waits for I/O completion
4. **Exception**: Kernel intervenes on fault

**Context Save/Restore**:
```asm
; Save context
PUSHAD              ; Save EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP
PUSH DS
PUSH ES
PUSH FS
PUSH GS

; Restore context
POP GS
POP FS
POP ES
POP DS
POPAD
IRET                ; Restore EIP and EFLAGS
```

---

## Device Drivers

### VGA Display Driver

**File**: [src/kernel/drivers/vga.c](src/kernel/drivers/vga.c)

**Hardware Details**:
- **Mode**: 80x25 text mode
- **Memory**: 0xB8000 - 0xB8FA0 (4 KB)
- **Character Size**: 2 bytes (ASCII + color attribute)

**Color Encoding**:
```
Byte 0: ASCII character
Byte 1: Color attribute
    Bits 3-0: Foreground (0-15)
    Bits 7-4: Background (0-7)
```

**Available Colors**:
```c
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
```

**Interface**:
```c
int vga_init(void)                          /* Initialize driver */
void vga_putchar(char ch, uint8_t color)    /* Write single char */
void vga_putstr(const char *str, uint8_t color)  /* Write string */
void vga_clear(void)                        /* Clear screen */
void vga_scroll_up(void)                    /* Scroll up one line */
void vga_move_cursor(uint8_t x, uint8_t y) /* Set cursor position */
void vga_get_cursor(uint8_t *x, uint8_t *y) /* Read cursor position */
```

**I/O Ports**:
```
0x3D4: CRT Controller Address Register
0x3D5: CRT Controller Data Register

To get cursor position:
  OUT 0x3D4, 0x0E  ; Select cursor high byte
  IN  0x3D5, AL    ; Read
  OUT 0x3D4, 0x0F  ; Select cursor low byte
  IN  0x3D5, AL    ; Read
```

### Keyboard Driver

**File**: [src/kernel/drivers/kbd.c](src/kernel/drivers/kbd.c)

**Hardware Details**:
- **Controller**: Intel 8042 PS/2 Controller
- **Interrupt**: IRQ1
- **I/O Port**: 0x60 (data), 0x64 (status/control)

**Scancode Processing**:
```c
typedef struct {
    uint8_t scancode;       /* Raw scancode from keyboard */
    uint8_t keycode;        /* Translated key code */
    char ascii;             /* ASCII character */
    int32_t pressed;        /* 1 if pressed, 0 if released */
} key_event_t;
```

**Scancode to ASCII Mapping**:
```
Scancode    Key     ASCII
────────    ───     ─────
0x02        1       '1'
0x03        2       '2'
...
0x1E        A       'a' or 'A'
...
0x3C        F2      (special)
0x01        ESC     0x1B
```

**Interface**:
```c
int kbd_init(void)                  /* Initialize keyboard driver */
int kbd_get_key(void)               /* Get key from buffer */
void kbd_buffer_push(key_event_t)   /* Add to buffer */
void kbd_handler(void)              /* IRQ1 handler (called by ISR) */
```

**Input Buffer**:
```c
#define KBD_BUFFER_SIZE     256

typedef struct {
    key_event_t buffer[KBD_BUFFER_SIZE];
    uint16_t head;      /* Write position */
    uint16_t tail;      /* Read position */
} kbd_buffer_t;
```

### Serial Port Driver

**File**: [src/kernel/drivers/serial.c](src/kernel/drivers/serial.c)

**Hardware Details**:
- **Port**: COM1 (0x3F8 - 0x3FF)
- **Baudrate**: 115200 bps
- **Format**: 8 data bits, no parity, 1 stop bit

**Register Layout**:
```
Port        Name                    Read/Write
────        ────                    ──────────
0x3F8       Data Register           RW
0x3F9       Interrupt Enable        W (when DLAB=0)
0x3FA       Interrupt Status        R
0x3FB       Line Control Register   W
0x3FC       Modem Control Register  W
0x3FD       Line Status Register    R
0x3FE       Modem Status Register   R
0x3FF       Scratch Register        RW

Note: 0x3F8-0x3F9 map to divisor when DLAB=1
```

**Interface**:
```c
int serial_init(void)               /* Initialize serial port */
void serial_putchar(char ch)         /* Write single character */
void serial_putstr(const char *str) /* Write string */
int serial_getchar(void)             /* Read character (blocking) */
```

---

## Shell Interface

**File**: [src/kernel/shell/shell.c](src/kernel/shell/shell.c)

The shell provides a command-line interface for system interaction.

### Shell Prompt

```
nimbu@localhost:~$
```

### Supported Commands

| Command | Syntax | Description |
|---------|--------|-------------|
| `help` | `help` | Display available commands |
| `echo` | `echo <text>` | Print text to screen |
| `clear` | `clear` | Clear screen |
| `mem` | `mem` | Display memory statistics |
| `ps` | `ps` | List running processes |
| `uname` | `uname [-a]` | System information |
| `reboot` | `reboot` | Reboot system |
| `ls` | `ls [dir]` | List files (stub implementation) |
| `info` | `info` | Detailed system information |

### Command Parsing

**Algorithm**:
```
1. Read input from keyboard
2. Parse line into tokens (split on spaces)
3. Get first token as command name
4. Match against known commands
5. Pass remaining tokens as arguments
6. Execute corresponding handler
7. Display result
8. Redisplay prompt
```

**Input Processing**:
```c
#define SHELL_BUFFER_SIZE       256
#define SHELL_MAX_ARGS          32

typedef struct {
    char line[SHELL_BUFFER_SIZE];       /* Current input line */
    int32_t pos;                        /* Current cursor position */
    char *argv[SHELL_MAX_ARGS];         /* Parsed arguments */
    int32_t argc;                       /* Argument count */
} shell_state_t;
```

### Memory Command Output

```
Memory Statistics:
┌──────────────────────────────────┐
│ Total Physical Memory: 128 MB    │
│ Free Memory: 64 MB               │
│ Used Memory: 64 MB               │
│ Heap Usage: 2 MB / 8 MB          │
└──────────────────────────────────┘
```

### Process Listing

```
PID   State     Runtime(ms)  Priority  Parent
───   ──────    ───────────  ────────  ──────
1     RUNNING   1543         NORMAL    0
2     READY     542          NORMAL    1
3     BLOCKED   234          LOW       1
```

---

## Build System

### Makefile Overview

**File**: [Makefile](Makefile)

The project uses GNU Make with cross-compiler toolchain.

### Required Tools

```bash
# Cross-compiler for i386
i686-elf-gcc         # C compiler
i686-elf-ld          # Linker
i686-elf-as / nasm   # Assembler

# Other tools
make                 # Build system
```

### Build Configuration

```makefile
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld

CFLAGS = -ffreestanding -nostdlib -fno-builtin \
         -Iinclude -Wall -Wextra
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -e _start
```

**Compiler Flags Explained**:
- `-ffreestanding`: Not linked to standard C library
- `-nostdlib`: Don't link standard library
- `-fno-builtin`: Don't optimize with built-in functions
- `-Iinclude`: Include directory
- `-Wall -Wextra`: All warnings

### Build Targets

```bash
make             # Build nimbu.bin and nimbu.elf
make clean       # Remove build artifacts
make run         # Run in QEMU
make debug       # Run in QEMU with GDB support
```

### Linking Script

**File**: [linker.ld](linker.ld)

The linker script defines memory sections and addresses.

**Key Sections**:
```ld
ENTRY(_start)
SECTIONS
{
    . = 0x100000;           /* Load at 1 MB */
    
    .boot : { *(.boot) }    /* Bootloader code */
    .text : { *(.text) }    /* Program code */
    .rodata : { *(.rodata) } /* Read-only data */
    .data : { *(.data) }    /* Initialized data */
    .bss : { *(.bss) }      /* Uninitialized data */
}
```

### Build Process

```
C Source Files → Compile → Object Files ─┐
                                          ├─ Link → ELF Binary
Assembly Files → Assemble → Object Files ┘
                                          │
                                          └─ Strip → Binary (for BIOS)
```

---

## API Reference

### Memory Management API

```c
/* Physical Memory Manager */
int pmm_init(uint32_t total_mem);
uint32_t pmm_alloc(void);                   /* Returns phys addr */
void pmm_free(uint32_t addr);
uint32_t pmm_get_free_pages(void);
uint32_t pmm_get_total_pages(void);

/* Virtual Memory Manager */
int vmm_init(void);
uint32_t vmm_create_page_dir(void);         /* Returns phys addr */
int vmm_map_page(uint32_t virt, uint32_t phys, int flags);
int vmm_unmap_page(uint32_t virt);
uint32_t vmm_get_physical(uint32_t virt);
void vmm_set_page_dir(uint32_t phys);

/* Kernel Heap */
void *kmalloc(size_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, size_t new_size);
void *kcalloc(size_t count, size_t size);
```

### Interrupt Handling API

```c
/* GDT */
int gdt_init(void);
int gdt_set_gate(int index, uint32_t base, uint32_t limit);
void gdt_load(void);

/* IDT */
int idt_init(void);
int idt_set_gate(int vector, void (*handler)(void), int dpl);
void idt_load(void);

/* Interrupts */
void sti(void);                  /* Enable interrupts */
void cli(void);                  /* Disable interrupts */
uint32_t save_flags(void);       /* Save EFLAGS */
void restore_flags(uint32_t);    /* Restore EFLAGS */
```

### Process Management API

```c
/* Process */
int proc_init(void);
uint32_t proc_create(void (*entry)(void), int priority);
void proc_exit(int exit_code);
uint32_t proc_get_current_pid(void);
pcb_t *proc_get_pcb(uint32_t pid);

/* Scheduler */
int scheduler_init(void);
void schedule(void);                /* Switch to next process */
void scheduler_tick(void);          /* Called by timer interrupt */
void switch_context(pcb_t *from, pcb_t *to);
```

### Driver APIs

```c
/* VGA Display */
int vga_init(void);
void vga_putchar(char ch, uint8_t color);
void vga_putstr(const char *str, uint8_t color);
void vga_clear(void);
void vga_scroll_up(void);
void vga_move_cursor(uint8_t x, uint8_t y);
void vga_get_cursor(uint8_t *x, uint8_t *y);

/* Keyboard */
int kbd_init(void);
int kbd_get_key(void);              /* Returns key code or -1 */
void kbd_handler(void);             /* ISR (called from int 0x21) */

/* Serial */
int serial_init(void);
void serial_putchar(char ch);
void serial_putstr(const char *str);
int serial_getchar(void);           /* Blocking read */
```

### Shell API

```c
/* Shell */
int shell_init(void);
int shell_run(void);                /* Main shell loop */
int shell_execute_command(const char *line);
void shell_print_prompt(void);
```

---

## Implementation Guide

### Adding a New System Call

1. **Define syscall number** in [include/interrupt.h](include/interrupt.h):
   ```c
   #define SYSCALL_READ    1
   #define SYSCALL_WRITE   2
   #define SYSCALL_EXIT    3
   #define SYSCALL_MMAP    4  /* New syscall */
   ```

2. **Implement handler** in [src/kernel/arch/x86/idt.c](src/kernel/arch/x86/idt.c):
   ```c
   void syscall_mmap(interrupt_context_t *ctx) {
       uint32_t addr = ctx->ebx;  /* First arg in EBX */
       uint32_t size = ctx->ecx;  /* Second arg in ECX */
       /* ... implementation ... */
       ctx->eax = result;         /* Return value in EAX */
   }
   ```

3. **Register in syscall handler**:
   ```c
   void syscall_handler(interrupt_context_t *ctx) {
       uint32_t syscall_num = ctx->eax;
       
       switch (syscall_num) {
           case SYSCALL_MMAP:
               syscall_mmap(ctx);
               break;
           /* ... */
       }
   }
   ```

### Adding a New Shell Command

1. **Add command handler** in [src/kernel/shell/shell.c](src/kernel/shell/shell.c):
   ```c
   static void cmd_newcommand(int argc, char *argv[]) {
       if (argc < 2) {
           vga_putstr("Usage: newcommand <arg>\n", VGA_COLOR_RED);
           return;
       }
       /* ... command logic ... */
   }
   ```

2. **Add to command table**:
   ```c
   static struct {
       const char *name;
       void (*handler)(int, char **);
   } commands[] = {
       {"help", cmd_help},
       {"echo", cmd_echo},
       {"newcommand", cmd_newcommand},  /* Add here */
       {NULL, NULL}
   };
   ```

### Adding a New Device Driver

1. **Create driver file**: `src/kernel/drivers/newdev.c`

2. **Create header**: `include/newdev.h` with interface:
   ```c
   int newdev_init(void);
   void newdev_write(uint8_t data);
   uint8_t newdev_read(void);
   ```

3. **Update Makefile** to compile new driver:
   ```makefile
   DRV_OBJS = $(OBJ_DIR)/vga.o $(OBJ_DIR)/kbd.o $(OBJ_DIR)/serial.o $(OBJ_DIR)/newdev.o
   
   $(OBJ_DIR)/newdev.o: $(DRV_SRC)/newdev.c | $(OBJ_DIR)
       $(CC) $(CFLAGS) -c $< -o $@
   ```

4. **Call initialization** in `kmain()`:
   ```c
   if (newdev_init()) {
       vga_putstr("newdev initialized\n", VGA_COLOR_GREEN);
   } else {
       vga_putstr("newdev initialization failed\n", VGA_COLOR_RED);
   }
   ```

---

## Configuration & Customization

### Memory Parameters

**File**: [include/mem.h](include/mem.h)

```c
/* Physical Memory */
#define PMM_MAX_PAGES           32768   /* 128 MB total */
#define PMM_PAGE_SIZE           4096    /* 4 KB pages */

/* Virtual Memory */
#define KERNEL_BASE             0xC0000000
#define HEAP_BASE               0xC0100000
#define HEAP_MAX_SIZE           0x800000   /* 8 MB */
#define PAGE_DIR_COUNT          1024
#define PAGE_TABLE_SIZE         1024
```

### Process Parameters

**File**: [include/process.h](include/process.h)

```c
#define MAX_PROCESSES           256
#define SCHEDULER_TIME_SLICE    10      /* 100 ms at 100 Hz */
#define PROC_PRIORITY_LOW       2
#define PROC_PRIORITY_NORMAL    1
#define PROC_PRIORITY_HIGH      0
```

### Timer Configuration

**File**: [src/kernel/arch/x86/idt.c](src/kernel/arch/x86/idt.c)

```c
#define PIT_FREQ                100      /* 100 Hz */
#define PIT_DIVISOR             (1193180 / PIT_FREQ)
```

To change to 1000 Hz:
```c
#define PIT_FREQ                1000
#define PIT_DIVISOR             (1193180 / 1000)  /* = 1193 */
```

### Shell Buffer Size

**File**: [src/kernel/shell/shell.c](src/kernel/shell/shell.c)

```c
#define SHELL_BUFFER_SIZE       256     /* Max input line length */
#define SHELL_MAX_ARGS          32      /* Max arguments */
#define SHELL_MAX_COMMANDS      16      /* Max command table entries */
```

---

## Troubleshooting

### Build Issues

#### Problem: `i686-elf-gcc: command not found`
**Solution**: Install cross-compiler
```bash
# Ubuntu/Debian
sudo apt-get install crossbuild-essential-i386

# Or download from: https://gcc.gnu.org/
```

#### Problem: `Makefile: No such file or directory`
**Solution**: Ensure Makefile is in project root
```bash
cd "NIMBU OS"
ls Makefile
```

#### Problem: `linker.ld: No such file or directory`
**Solution**: Verify linker script path in Makefile
```makefile
LDFLAGS = -T linker.ld -e _start  # Path relative to Makefile
```

### Runtime Issues

#### Problem: System crashes immediately after boot
**Possible Causes**:
1. Stack overflow - check stack size in boot.asm
2. GDT/IDT initialization failed - verify in kernel.c output
3. Memory addressing - check page directory physical address

**Debug Steps**:
1. Add debug output before each initialization
2. Check serial port output with `serial_putstr()`
3. Run in QEMU with `-debug` flag

#### Problem: Keyboard input not working
**Possible Causes**:
1. IRQ1 handler not registered
2. Keyboard buffer overflow
3. PS/2 controller reset required

**Debug Steps**:
```bash
# Test with serial port instead:
serial_putstr("Testing keyboard...\n");

# Check interrupt vector:
# 32 (default) + 1 (IRQ1) = 0x21
```

#### Problem: Memory leak or corruption
**Debug Steps**:
1. Add size tracking to heap allocations
2. Use `cmd_mem()` to check heap statistics
3. Add boundary markers to allocated blocks:
   ```c
   #define ALLOC_GUARD_VALUE   0xDEADBEEF
   ```

### Testing

#### Minimal Test Program

```c
/* test.c - Simple test for process creation */
void test_process(void) {
    vga_putstr("Creating process...\n", VGA_COLOR_GREEN);
    uint32_t pid = proc_create(test_function, PROC_PRIORITY_NORMAL);
    vga_putstr("Process created with PID: ", VGA_COLOR_GREEN);
    /* print pid */
    vga_putstr("\n", VGA_COLOR_GREEN);
}

void test_function(void) {
    int i = 0;
    while (1) {
        i++;
        if (i % 1000 == 0) {
            vga_putstr(".", VGA_COLOR_CYAN);
        }
    }
}
```

### Performance Tuning

#### Timer Frequency

Increase frequency for more responsive scheduling (higher overhead):
```c
#define PIT_FREQ                1000    /* 1 kHz instead of 100 Hz */
```

#### Memory Block Size

Adjust page size for memory fragmentation vs TLB efficiency:
```c
/* Currently 4 KB - good balance for educational OS */
/* Larger pages = less paging overhead but more waste */
```

---

## File Structure Reference

### Header Files (`include/`)

| File | Purpose |
|------|---------|
| `types.h` | Basic type definitions |
| `mem.h` | Memory management interface |
| `interrupt.h` | Interrupt handling interface |
| `process.h` | Process management interface |
| `driver.h` | Device driver interfaces |
| `shell.h` | Shell interface |
| `gdt.h` | GDT interface |

### Source Files (`src/`)

```
src/
├── boot/boot.asm              # Bootloader entry point
└── kernel/
    ├── kernel.c               # Main kernel initialization
    ├── arch/x86/
    │   ├── gdt.c             # Global Descriptor Table
    │   └── idt.c             # Interrupt Descriptor Table
    ├── mm/
    │   ├── pmm.c             # Physical Memory Manager
    │   ├── vmm.c             # Virtual Memory Manager
    │   └── heap.c            # Dynamic Heap Allocator
    ├── proc/
    │   ├── process.c         # Process management
    │   └── scheduler.c       # Process scheduler
    ├── drivers/
    │   ├── vga.c             # VGA text display driver
    │   ├── kbd.c             # PS/2 keyboard driver
    │   └── serial.c          # Serial port driver
    └── shell/
        └── shell.c           # Command-line shell
```

### Build Files

| File | Purpose |
|------|---------|
| `Makefile` | Build configuration and targets |
| `linker.ld` | Memory layout and section definitions |
| `scripts/build.sh` | Convenience build script |

### Documentation Files

| File | Purpose |
|------|---------|
| `README.md` | Project overview and quick start |
| `ARCHITECTURE.md` | High-level architecture |
| `SOURCE_CODE_SUMMARY.md` | Code statistics and features |
| `DOCUMENTATION.md` | This comprehensive technical guide |
| `FILE_LISTING.md` | Complete file inventory |

---

## Quick Reference

### Common Macros

```c
#define NULL                0
#define TRUE                1
#define FALSE               0

#define container_of(ptr, type, member) \
    ((type *)((void *)(ptr) - offsetof(type, member)))

#define ALIGN_UP(x, align)   (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
```

### Important Addresses

```
0x00000000  Real Mode IVT
0x00000400  BIOS Data Area
0x00001000  Kernel heap start (PMM bitmap)
0x00100000  Kernel entry point (physical)
0x3F8       Serial port COM1
0xB8000     VGA video memory
0xC0000000  Kernel base (virtual)
0xC0100000  Kernel heap (virtual)
```

### Key Register Usage (32-bit x86)

```
EAX/RAX     Accumulator (function returns)
EBX/RBX     Base index
ECX/RCX     Counter (loop, string ops)
EDX/RDX     Data register
ESI/RSI     Source index (strings)
EDI/RDI     Destination index (strings)
EBP/RBP     Stack base pointer
ESP/RSP     Stack pointer

CS          Code segment
DS          Data segment
ES          Extra segment
FS/GS       Additional segments
SS          Stack segment

EIP         Instruction pointer
EFLAGS      Flags register
CR0         Control register 0 (paging bit 31)
CR3         Control register 3 (page dir base)
GDTR        GDT register
IDTR        IDT register
```

---

## References & Further Reading

### x86 Documentation
- Intel 64 and IA-32 Architectures Software Developer's Manual (https://www.intel.com/content/www/en/en/download/758159.html)
- AMD64 Architecture Programmer's Manual (https://www.amd.com/system/files/TechDocs/24592.pdf)

### Operating Systems
- "Operating System Concepts" by Silberschatz, Galvin, Gagne
- "The Linux Kernel" by various Linux kernel developers
- "xv6: A simple Unix-like teaching operating system" (MIT)

### Bootloaders
- Multiboot Specification (https://www.gnu.org/software/grub/manual/multiboot/html_node/)
- OSDev Wiki (https://wiki.osdev.org/)

### Tools & Utilities
- GCC (GNU Compiler Collection): https://gcc.gnu.org/
- QEMU Emulator: https://www.qemu.org/
- GDB Debugger: https://www.gnu.org/software/gdb/

---

**Documentation Version**: 1.0  
**Last Updated**: May 30, 2026  
**NIMBU OS Project**

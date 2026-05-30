# NIMBU OS - Complete API Reference

**Function, Structure, and Macro Reference**

---

## Table of Contents

1. [Memory Management API](#memory-management-api)
2. [Interrupt & CPU API](#interrupt--cpu-api)
3. [Process Management API](#process-management-api)
4. [VGA Display API](#vga-display-api)
5. [Keyboard API](#keyboard-api)
6. [Serial Port API](#serial-port-api)
7. [Shell API](#shell-api)
8. [Type Definitions](#type-definitions)
9. [Macros & Constants](#macros--constants)
10. [Data Structures](#data-structures)

---

## Memory Management API

### Physical Memory Manager (PMM)

**Header**: [include/mem.h](include/mem.h)  
**Implementation**: [src/kernel/mm/pmm.c](src/kernel/mm/pmm.c)

#### Functions

##### `int pmm_init(uint32_t total_memory)`

Initialize the physical memory manager.

**Parameters**:
- `total_memory`: Total RAM size in bytes

**Returns**:
- `0`: Success
- `-1`: Failure (insufficient memory for bitmap)

**Description**:
Creates a bitmap to track allocated/free 4 KB pages. Must be called before any `pmm_alloc()` calls.

**Example**:
```c
if (pmm_init(128 * 1024 * 1024)) {  /* 128 MB */
    vga_putstr("PMM initialized\n", VGA_COLOR_GREEN);
} else {
    vga_putstr("PMM init failed\n", VGA_COLOR_RED);
}
```

---

##### `uint32_t pmm_alloc(void)`

Allocate a single 4 KB physical page.

**Parameters**: None

**Returns**:
- Physical address of allocated page
- `0xFFFFFFFF`: Failure (no free pages)

**Description**:
Finds first free page in bitmap and marks as allocated. Returns physical address.

**Example**:
```c
uint32_t phys_page = pmm_alloc();
if (phys_page != 0xFFFFFFFF) {
    vga_putstr("Allocated page: ", VGA_COLOR_GREEN);
    /* print phys_page */
}
```

**Notes**:
- Non-blocking
- O(n) complexity where n = total pages
- May fail if no free pages available

---

##### `void pmm_free(uint32_t phys_addr)`

Free a previously allocated physical page.

**Parameters**:
- `phys_addr`: Physical address of page to free (must be page-aligned)

**Returns**: None

**Description**:
Marks the page as free in the bitmap for reallocation.

**Example**:
```c
pmm_free(phys_addr);
vga_putstr("Page freed\n", VGA_COLOR_GREEN);
```

**Warning**:
- Double-free is not detected; can cause corruption
- Freeing invalid address may corrupt PMM bitmap

---

##### `uint32_t pmm_get_free_pages(void)`

Get number of currently free pages.

**Parameters**: None

**Returns**: Number of free 4 KB pages

**Example**:
```c
uint32_t free_pages = pmm_get_free_pages();
uint32_t free_mb = (free_pages * 4096) / (1024 * 1024);
```

---

##### `uint32_t pmm_get_total_pages(void)`

Get total number of pages managed.

**Parameters**: None

**Returns**: Total number of pages

**Example**:
```c
uint32_t total = pmm_get_total_pages();
vga_putstr("Total pages: ", VGA_COLOR_CYAN);
/* print total */
```

---

### Virtual Memory Manager (VMM)

**Header**: [include/mem.h](include/mem.h)  
**Implementation**: [src/kernel/mm/vmm.c](src/kernel/mm/vmm.c)

#### Functions

##### `int vmm_init(void)`

Initialize virtual memory and paging.

**Parameters**: None

**Returns**:
- `0`: Success
- `-1`: Failure

**Description**:
Sets up page directory structure and enables paging through CR0. Must be called after PMM is initialized.

**Example**:
```c
if (vmm_init()) {
    vga_putstr("Paging enabled\n", VGA_COLOR_GREEN);
}
```

**Notes**:
- Sets up identity mapping for first 1 MB
- Creates higher-half kernel mapping (0xC0000000+)
- Enables bit 31 of CR0 (paging bit)

---

##### `uint32_t vmm_create_page_dir(void)`

Create a new page directory for a process.

**Parameters**: None

**Returns**:
- Physical address of new page directory
- `0xFFFFFFFF`: Failure (no memory)

**Description**:
Allocates a new page directory (4 KB) for process virtual address space. Kernel entries are shared.

**Example**:
```c
uint32_t page_dir = vmm_create_page_dir();
if (page_dir != 0xFFFFFFFF) {
    /* Use for new process */
}
```

---

##### `int vmm_map_page(uint32_t virt, uint32_t phys, int flags)`

Map a virtual address to physical address.

**Parameters**:
- `virt`: Virtual address (must be page-aligned)
- `phys`: Physical address (must be page-aligned)
- `flags`: Page flags (Present, Writable, User/Supervisor, etc.)

**Returns**:
- `0`: Success
- `-1`: Failure (memory allocation or invalid address)

**Description**:
Creates page table entry mapping virtual to physical. Allocates intermediate page table if needed.

**Example**:
```c
int flags = PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
vmm_map_page(0x1000, 0x2000, flags);
```

**Flags**:
```c
#define PAGE_PRESENT        0x1
#define PAGE_WRITABLE       0x2
#define PAGE_USER           0x4
#define PAGE_WRITE_THROUGH  0x8
#define PAGE_CACHE_DISABLE  0x10
#define PAGE_ACCESSED       0x20
#define PAGE_DIRTY          0x40
#define PAGE_PSE            0x80
#define PAGE_GLOBAL         0x100
```

---

##### `int vmm_unmap_page(uint32_t virt)`

Unmap a virtual address.

**Parameters**:
- `virt`: Virtual address to unmap

**Returns**:
- `0`: Success
- `-1`: Failure (page not mapped)

**Description**:
Removes page table entry and invalidates TLB.

**Example**:
```c
vmm_unmap_page(0x1000);
```

---

##### `uint32_t vmm_get_physical(uint32_t virt)`

Translate virtual address to physical address.

**Parameters**:
- `virt`: Virtual address

**Returns**:
- Physical address if mapped
- `0xFFFFFFFF`: Not mapped

**Example**:
```c
uint32_t phys = vmm_get_physical(0xC0000000);
if (phys != 0xFFFFFFFF) {
    /* Successfully mapped */
}
```

---

##### `void vmm_set_page_dir(uint32_t phys)`

Switch active page directory (CR3).

**Parameters**:
- `phys`: Physical address of page directory

**Returns**: None

**Description**:
Loads page directory into CR3. Flushes TLB. Used in context switching.

**Example**:
```c
vmm_set_page_dir(pcb->page_dir_phys);  /* Switch process memory */
```

---

### Kernel Heap

**Header**: [include/mem.h](include/mem.h)  
**Implementation**: [src/kernel/mm/heap.c](src/kernel/mm/heap.c)

#### Functions

##### `void *kmalloc(size_t size)`

Allocate memory from kernel heap.

**Parameters**:
- `size`: Number of bytes to allocate

**Returns**:
- Virtual address of allocated block
- `NULL`: Failure (insufficient memory)

**Description**:
First-fit allocation with block metadata. Allocates extra space for metadata.

**Example**:
```c
char *buffer = (char *)kmalloc(1024);
if (buffer != NULL) {
    buffer[0] = 'H';
}
```

**Notes**:
- Header stored immediately before returned pointer
- Returned pointer is 4-byte aligned
- Size includes metadata overhead (~8 bytes)

---

##### `void kfree(void *ptr)`

Free previously allocated memory.

**Parameters**:
- `ptr`: Pointer returned by kmalloc

**Returns**: None

**Description**:
Marks block as free. Merges adjacent free blocks to reduce fragmentation.

**Example**:
```c
kfree(buffer);
```

**Warning**:
- Double-free causes corruption
- Must use exact pointer returned by kmalloc

---

##### `void *krealloc(void *ptr, size_t new_size)`

Resize allocated memory block.

**Parameters**:
- `ptr`: Current pointer (or NULL)
- `new_size`: New size in bytes

**Returns**:
- New pointer
- `NULL`: Failure

**Description**:
Allocates new block, copies data, frees old block. Slow operation - minimize use.

**Example**:
```c
char *new_ptr = (char *)krealloc(ptr, 2048);
if (new_ptr != NULL) {
    ptr = new_ptr;
}
```

---

##### `void *kcalloc(size_t count, size_t size)`

Allocate and zero memory.

**Parameters**:
- `count`: Number of elements
- `size`: Size of each element

**Returns**:
- Virtual address of allocated block (zeroed)
- `NULL`: Failure

**Example**:
```c
int *array = (int *)kcalloc(100, sizeof(int));  /* 100 ints, zeroed */
```

---

## Interrupt & CPU API

### Global Descriptor Table (GDT)

**Header**: [include/gdt.h](include/gdt.h)  
**Implementation**: [src/kernel/arch/x86/gdt.c](src/kernel/arch/x86/gdt.c)

#### Functions

##### `int gdt_init(void)`

Initialize Global Descriptor Table.

**Parameters**: None

**Returns**:
- `0`: Success
- `-1`: Failure

**Description**:
Creates GDT entries for kernel code/data and user code/data segments. Sets up flat 4 GB model.

**Example**:
```c
if (gdt_init()) {
    vga_putstr("GDT initialized\n", VGA_COLOR_GREEN);
}
```

---

##### `int gdt_set_gate(int index, uint32_t base, uint32_t limit)`

Add/modify GDT entry.

**Parameters**:
- `index`: GDT entry index (0-5 reserved by kernel)
- `base`: Segment base address
- `limit`: Segment limit

**Returns**:
- `0`: Success
- `-1`: Invalid index

**Example**:
```c
/* Already done by gdt_init(), shown for reference */
gdt_set_gate(1, 0x0, 0xFFFFFFFF);  /* Kernel code */
gdt_set_gate(2, 0x0, 0xFFFFFFFF);  /* Kernel data */
```

---

##### `void gdt_load(void)`

Load GDT into GDTR register.

**Parameters**: None

**Returns**: None

**Description**:
Executes LGDT instruction. Called after setting up GDT entries.

**Example**:
```c
gdt_init();
gdt_load();  /* Now CPU uses new GDT */
```

---

### Interrupt Descriptor Table (IDT)

**Header**: [include/interrupt.h](include/interrupt.h)  
**Implementation**: [src/kernel/arch/x86/idt.c](src/kernel/arch/x86/idt.c)

#### Functions

##### `int idt_init(void)`

Initialize Interrupt Descriptor Table.

**Parameters**: None

**Returns**:
- `0`: Success
- `-1`: Failure

**Description**:
Creates 256 IDT entries, sets up exception handlers (0-31) and IRQ handlers (32-47). Syscall handler at 128.

**Example**:
```c
if (idt_init()) {
    vga_putstr("IDT initialized\n", VGA_COLOR_GREEN);
}
```

---

##### `int idt_set_gate(int vector, void (*handler)(void), int dpl)`

Add interrupt handler to IDT.

**Parameters**:
- `vector`: Interrupt vector number (0-255)
- `handler`: Pointer to handler function
- `dpl`: Descriptor Privilege Level (0=kernel, 3=user)

**Returns**:
- `0`: Success
- `-1`: Invalid vector

**Example**:
```c
idt_set_gate(0x80, syscall_handler, 3);  /* User-callable syscall */
```

---

##### `void idt_load(void)`

Load IDT into IDTR register.

**Parameters**: None

**Returns**: None

**Description**:
Executes LIDT instruction. Called after setting up IDT entries.

**Example**:
```c
idt_init();
idt_load();  /* Now CPU uses new IDT */
```

---

### CPU Flags & Interrupts

**Header**: [include/interrupt.h](include/interrupt.h)

#### Inline Functions

##### `void sti(void)`

Enable interrupts.

**Description**: Sets IF flag in EFLAGS.

**Example**:
```c
sti();  /* Allow hardware interrupts */
```

---

##### `void cli(void)`

Disable interrupts.

**Description**: Clears IF flag in EFLAGS.

**Example**:
```c
cli();  /* Prevent hardware interrupts */
```

---

##### `uint32_t save_flags(void)`

Save current interrupt state.

**Returns**: Current EFLAGS value

**Example**:
```c
uint32_t flags = save_flags();
cli();  /* Disable interrupts */
/* Critical section */
restore_flags(flags);  /* Restore original state */
```

---

##### `void restore_flags(uint32_t flags)`

Restore interrupt state.

**Parameters**:
- `flags`: EFLAGS value from save_flags()

**Example**:
```c
uint32_t flags = save_flags();
/* ... do something ... */
restore_flags(flags);
```

---

## Process Management API

### Process Control

**Header**: [include/process.h](include/process.h)  
**Implementation**: [src/kernel/proc/process.c](src/kernel/proc/process.c)

#### Functions

##### `int proc_init(void)`

Initialize process management system.

**Parameters**: None

**Returns**:
- `0`: Success
- `-1`: Failure

**Description**:
Initializes process table, scheduler, and creates first process (kernel).

**Example**:
```c
if (proc_init()) {
    vga_putstr("Process management ready\n", VGA_COLOR_GREEN);
}
```

---

##### `uint32_t proc_create(void (*entry)(void), int priority)`

Create new process.

**Parameters**:
- `entry`: Entry point function (void func(void))
- `priority`: Priority level (PROC_PRIORITY_LOW, NORMAL, HIGH)

**Returns**:
- Process ID (PID) if success
- `0xFFFFFFFF`: Failure

**Description**:
Allocates PCB, page directory, stacks. Adds to ready queue.

**Example**:
```c
void process_main(void) {
    while (1) {
        vga_putstr(".", VGA_COLOR_GREEN);
    }
}

uint32_t pid = proc_create(process_main, PROC_PRIORITY_NORMAL);
```

---

##### `void proc_exit(int exit_code)`

Terminate current process.

**Parameters**:
- `exit_code`: Exit status (currently unused)

**Returns**: None (does not return)

**Description**:
Marks process as ZOMBIE, switches to next process.

**Example**:
```c
if (error_condition) {
    proc_exit(1);  /* Does not return */
}
```

---

##### `uint32_t proc_get_current_pid(void)`

Get PID of currently running process.

**Parameters**: None

**Returns**: PID of current process

**Example**:
```c
uint32_t my_pid = proc_get_current_pid();
```

---

##### `pcb_t *proc_get_pcb(uint32_t pid)`

Get Process Control Block.

**Parameters**:
- `pid`: Process ID

**Returns**:
- Pointer to PCB if found
- `NULL`: Process not found

**Example**:
```c
pcb_t *pcb = proc_get_pcb(5);
if (pcb != NULL) {
    vga_putstr("Process state: ", VGA_COLOR_CYAN);
    /* print pcb->state */
}
```

---

### Scheduler

**Header**: [include/process.h](include/process.h)  
**Implementation**: [src/kernel/proc/scheduler.c](src/kernel/proc/scheduler.c)

#### Functions

##### `int scheduler_init(void)`

Initialize scheduler.

**Parameters**: None

**Returns**:
- `0`: Success
- `-1`: Failure

**Description**:
Sets up timer interrupt for scheduling. Called during kernel init.

**Example**:
```c
if (scheduler_init()) {
    vga_putstr("Scheduler running at 100 Hz\n", VGA_COLOR_GREEN);
}
```

---

##### `void schedule(void)`

Find next process and switch to it.

**Parameters**: None

**Returns**: None

**Description**:
Implements round-robin selection. Saves current process state, loads next.

**Note**: Usually called from timer interrupt (IRQ0), not directly.

---

##### `void scheduler_tick(void)`

Called by timer interrupt.

**Parameters**: None

**Returns**: None

**Description**:
Decrements current process time slice. Calls schedule() if expired.

**Example**: (called from timer IRQ handler)
```c
/* IRQ0 handler */
timer_handler() {
    scheduler_tick();
}
```

---

##### `void switch_context(pcb_t *from, pcb_t *to)`

Perform context switch.

**Parameters**:
- `from`: Current process PCB
- `to`: Next process PCB

**Returns**: None

**Description**:
Saves CPU state from `from`, loads state into `to`. Called by scheduler.

**Note**: Usually called from assembly code or scheduler, rarely directly.

---

## VGA Display API

**Header**: [include/driver.h](include/driver.h)  
**Implementation**: [src/kernel/drivers/vga.c](src/kernel/drivers/vga.c)

### Functions

##### `int vga_init(void)`

Initialize VGA display driver.

**Parameters**: None

**Returns**:
- `0`: Success
- `-1`: Failure

**Description**:
Sets up 80x25 text mode, clears screen, positions cursor at (0,0).

**Example**:
```c
if (vga_init()) {
    vga_putstr("VGA ready\n", VGA_COLOR_WHITE);
}
```

---

##### `void vga_putchar(char ch, uint8_t color)`

Write single character.

**Parameters**:
- `ch`: Character to display
- `color`: Color attribute

**Returns**: None

**Description**:
Writes character at cursor position, advances cursor. Scrolls if needed.

**Example**:
```c
vga_putchar('H', VGA_COLOR_GREEN);
vga_putchar('i', VGA_COLOR_GREEN);
```

---

##### `void vga_putstr(const char *str, uint8_t color)`

Write string.

**Parameters**:
- `str`: Null-terminated string
- `color`: Color attribute

**Returns**: None

**Description**:
Calls vga_putchar for each character.

**Example**:
```c
vga_putstr("NIMBU OS started\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
```

---

##### `void vga_clear(void)`

Clear screen.

**Parameters**: None

**Returns**: None

**Description**:
Fills entire screen with spaces, resets cursor to (0,0).

**Example**:
```c
vga_clear();
```

---

##### `void vga_scroll_up(void)`

Scroll screen up one line.

**Parameters**: None

**Returns**: None

**Description**:
Shifts all lines up, clears bottom line, cursor moves up.

**Note**: Usually called automatically when printing past line 25.

---

##### `void vga_move_cursor(uint8_t x, uint8_t y)`

Set cursor position.

**Parameters**:
- `x`: Column (0-79)
- `y`: Row (0-24)

**Returns**: None

**Example**:
```c
vga_move_cursor(0, 10);  /* Move to row 10, col 0 */
vga_putstr("Cursor here\n", VGA_COLOR_WHITE);
```

---

##### `void vga_get_cursor(uint8_t *x, uint8_t *y)`

Get current cursor position.

**Parameters**:
- `x`: Output pointer for column
- `y`: Output pointer for row

**Returns**: None

**Example**:
```c
uint8_t x, y;
vga_get_cursor(&x, &y);
vga_putstr("Cursor at: ", VGA_COLOR_CYAN);
/* print x, y */
```

---

### Color Macros

##### `VGA_COLOR(fg, bg)`

Create color attribute byte.

**Parameters**:
- `fg`: Foreground color (0-15)
- `bg`: Background color (0-7)

**Returns**: Combined color byte

**Example**:
```c
uint8_t red_on_white = VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_WHITE);
vga_putstr("Error!\n", red_on_white);
```

---

### Color Constants

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

---

## Keyboard API

**Header**: [include/driver.h](include/driver.h)  
**Implementation**: [src/kernel/drivers/kbd.c](src/kernel/drivers/kbd.c)

### Functions

##### `int kbd_init(void)`

Initialize keyboard driver.

**Parameters**: None

**Returns**:
- `0`: Success
- `-1`: Failure

**Description**:
Initializes PS/2 controller, input buffer, registers IRQ1 handler.

**Example**:
```c
if (kbd_init()) {
    vga_putstr("Keyboard ready\n", VGA_COLOR_GREEN);
}
```

---

##### `int kbd_get_key(void)`

Get character from input buffer.

**Parameters**: None

**Returns**:
- ASCII character (0-255) if available
- `-1`: Buffer empty

**Description**:
Non-blocking read from keyboard buffer. Returns next character or -1.

**Example**:
```c
int ch = kbd_get_key();
if (ch != -1) {
    vga_putchar(ch, VGA_COLOR_WHITE);
}
```

---

##### `void kbd_handler(void)`

Keyboard interrupt handler.

**Parameters**: None

**Returns**: None

**Note**: Called automatically by IRQ1, not called directly.

---

### Scancode Mappings

Common scancodes (set 1):

```c
0x02-0x0B   '1'-'9' (top row numbers)
0x0C        '0'
0x0D        '-'
0x0E        '='

0x10-0x19   'QWERTYUIOP'
0x1A        '['
0x1B        ']'

0x1E-0x27   'ASDFGHJKL'
0x28        ';'
0x2B        '\\'

0x2C-0x35   'ZXCVBNM'
0x36        ','
0x37        '.'
0x38        '/'

0x39        SPACE
0x01        ESC
0x0F        TAB
0x1C        ENTER
0x3B-0x44   F1-F12
0x46        SCROLL LOCK
```

---

## Serial Port API

**Header**: [include/driver.h](include/driver.h)  
**Implementation**: [src/kernel/drivers/serial.c](src/kernel/drivers/serial.c)

### Functions

##### `int serial_init(void)`

Initialize serial port (COM1).

**Parameters**: None

**Returns**:
- `0`: Success
- `-1`: Failure

**Description**:
Configures UART at 115200 baud, 8 bits, no parity, 1 stop bit.

**Example**:
```c
if (serial_init()) {
    serial_putstr("Serial port ready\n");
}
```

---

##### `void serial_putchar(char ch)`

Write single character to serial port.

**Parameters**:
- `ch`: Character to transmit

**Returns**: None

**Description**:
Waits until transmit buffer ready, sends character.

**Example**:
```c
serial_putchar('A');
serial_putchar('\n');
```

---

##### `void serial_putstr(const char *str)`

Write string to serial port.

**Parameters**:
- `str`: Null-terminated string

**Returns**: None

**Example**:
```c
serial_putstr("Debug output here\n");
```

---

##### `int serial_getchar(void)`

Read character from serial port.

**Parameters**: None

**Returns**:
- Character read (0-255)
- `-1`: No data available

**Description**:
Non-blocking read. Returns data if available, -1 otherwise.

**Example**:
```c
int ch = serial_getchar();
if (ch != -1) {
    vga_putchar(ch, VGA_COLOR_WHITE);
}
```

---

### Serial Registers (I/O Ports)

```c
#define SERIAL_COM1         0x3F8

#define SERIAL_DATA         0      /* RW: Data register */
#define SERIAL_INTR_EN      1      /* RW: Interrupt enable */
#define SERIAL_INTR_ID      2      /* RO: Interrupt ID */
#define SERIAL_LINE_CTRL    3      /* RW: Line control */
#define SERIAL_MODEM_CTRL   4      /* RW: Modem control */
#define SERIAL_LINE_STAT    5      /* RO: Line status */
#define SERIAL_MODEM_STAT   6      /* RO: Modem status */
#define SERIAL_SCRATCH      7      /* RW: Scratch */
```

---

## Shell API

**Header**: [include/shell.h](include/shell.h)  
**Implementation**: [src/kernel/shell/shell.c](src/kernel/shell/shell.c)

### Functions

##### `int shell_init(void)`

Initialize shell.

**Parameters**: None

**Returns**:
- `0`: Success
- `-1`: Failure

**Description**:
Initializes command table, input buffer.

**Example**:
```c
if (shell_init()) {
    vga_putstr("Shell ready\n", VGA_COLOR_GREEN);
}
```

---

##### `int shell_run(void)`

Run main shell loop.

**Parameters**: None

**Returns**: Does not return (infinite loop)

**Description**:
Main shell loop. Displays prompt, reads input, parses and executes commands.

**Example**:
```c
shell_init();
shell_run();  /* Never returns */
```

---

##### `int shell_execute_command(const char *line)`

Execute single command.

**Parameters**:
- `line`: Command line string

**Returns**:
- `0`: Success
- `-1`: Unknown command

**Example**:
```c
shell_execute_command("echo Hello World");
```

---

##### `void shell_print_prompt(void)`

Display command prompt.

**Parameters**: None

**Returns**: None

**Example**:
```c
shell_print_prompt();  /* Prints: nimbu@localhost:~$ */
```

---

### Built-in Commands

| Command | Arguments | Description |
|---------|-----------|-------------|
| `help` | (none) | List available commands |
| `echo` | `<text>` | Print text |
| `clear` | (none) | Clear screen |
| `mem` | (none) | Show memory stats |
| `ps` | (none) | List processes |
| `uname` | `[-a]` | System info |
| `reboot` | (none) | Restart system |
| `info` | (none) | Detailed info |

---

## Type Definitions

**File**: [include/types.h](include/types.h)

### Integer Types

```c
typedef unsigned char      uint8_t;
typedef signed char        int8_t;
typedef unsigned short     uint16_t;
typedef signed short       int16_t;
typedef unsigned int       uint32_t;
typedef signed int         int32_t;
typedef unsigned long      ulong_t;
typedef signed long        long_t;
```

### Pointer Types

```c
typedef uint32_t           uintptr_t;
typedef int32_t            intptr_t;
```

### Boolean Type

```c
typedef int                bool_t;
#define TRUE               1
#define FALSE              0
```

### Memory Types

```c
typedef uint32_t           size_t;
typedef int32_t            ssize_t;
typedef uint32_t           phys_addr_t;
typedef uint32_t           virt_addr_t;
```

---

## Macros & Constants

### Memory Configuration

```c
/* Physical Memory Manager */
#define PMM_MAX_PAGES           32768   /* 128 MB */
#define PMM_PAGE_SIZE           4096    /* 4 KB */
#define PMM_PAGE_SHIFT          12      /* log2(4096) */

/* Virtual Memory */
#define KERNEL_BASE             0xC0000000
#define HEAP_BASE               0xC0100000
#define HEAP_MAX_SIZE           0x800000    /* 8 MB */
#define PAGE_DIR_COUNT          1024
#define PAGE_TABLE_SIZE         1024
```

### Paging Flags

```c
#define PAGE_PRESENT            0x1
#define PAGE_WRITABLE           0x2
#define PAGE_USER               0x4
#define PAGE_WRITE_THROUGH      0x8
#define PAGE_CACHE_DISABLE      0x10
#define PAGE_ACCESSED           0x20
#define PAGE_DIRTY              0x40
#define PAGE_PSE                0x80
#define PAGE_GLOBAL             0x100
```

### Interrupt Vectors

```c
#define ISR_DIVIDE_BY_ZERO      0
#define ISR_DEBUG               1
#define ISR_NMI                 2
#define ISR_BREAKPOINT          3
/* ... 0-31 are exceptions ... */

#define IRQ_TIMER               32
#define IRQ_KEYBOARD            33
/* ... 32-47 are hardware IRQs ... */

#define SYSCALL_VECTOR          128     /* 0x80 */
```

### Process States

```c
#define PROC_STATE_READY        0
#define PROC_STATE_RUNNING      1
#define PROC_STATE_BLOCKED      2
#define PROC_STATE_ZOMBIE       3
```

### Process Priorities

```c
#define PROC_PRIORITY_HIGH      0
#define PROC_PRIORITY_NORMAL    1
#define PROC_PRIORITY_LOW       2
```

### Scheduling

```c
#define MAX_PROCESSES           256
#define SCHEDULER_TIME_SLICE    10      /* 100 ms at 100 Hz */
#define SCHEDULER_FREQ          100     /* 100 Hz */
```

### VGA Text Mode

```c
#define VGA_WIDTH               80
#define VGA_HEIGHT              25
#define VGA_BUFFER              0xB8000
#define VGA_BUFFER_SIZE         0x4000  /* 16 KB */
```

### Keyboard

```c
#define KBD_BUFFER_SIZE         256
#define KBD_DATA_PORT           0x60
#define KBD_CTRL_PORT           0x64
```

### Serial

```c
#define SERIAL_COM1             0x3F8
#define SERIAL_BAUD             115200
```

### Shell

```c
#define SHELL_BUFFER_SIZE       256
#define SHELL_MAX_ARGS          32
#define SHELL_MAX_COMMANDS      16
```

---

## Data Structures

### Process Control Block (PCB)

**File**: [include/process.h](include/process.h)

```c
typedef struct {
    uint32_t pid;                       /* Process ID */
    uint32_t parent_pid;                /* Parent PID */
    uint32_t state;                     /* Process state */
    uint32_t priority;                  /* Priority level */
    
    interrupt_context_t *context;       /* Saved registers */
    
    uint32_t page_dir_phys;             /* Page directory phys addr */
    uint32_t kernel_stack;              /* Kernel stack pointer */
    uint32_t user_stack;                /* User stack pointer */
    
    uint32_t time_slice_remaining;      /* Ticks left in timeslice */
    uint32_t total_runtime;             /* Total runtime ticks */
    uint32_t wait_queue;                /* Wait queue if blocked */
    
    void *blocked_on;                   /* Blocked on resource */
} pcb_t;
```

### Interrupt Context

**File**: [include/interrupt.h](include/interrupt.h)

```c
typedef struct {
    /* General purpose registers */
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp, esp;
    
    /* Control registers */
    uint32_t eip, eflags;
    
    /* Segment registers */
    uint32_t cs, ss, ds, es, fs, gs;
    
    /* Error code (for some exceptions) */
    uint32_t error_code;
} interrupt_context_t;
```

### GDT Entry

**File**: [src/kernel/arch/x86/gdt.c](src/kernel/arch/x86/gdt.c)

```c
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t flags;
    uint8_t limit_high;
    uint8_t base_high;
} gdt_entry_t;
```

### Heap Block

**File**: [src/kernel/mm/heap.c](src/kernel/mm/heap.c)

```c
typedef struct heap_block {
    uint32_t size;                      /* Block size */
    struct heap_block *next;            /* Next block */
    int32_t free;                       /* Free flag */
    /* Data follows immediately */
} heap_block_t;
```

---

**API Reference Version**: 1.0  
**Last Updated**: May 30, 2026

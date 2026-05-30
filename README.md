# NIMBU OS - Minimal Linux Kernel

**A Minimal Linux-Based Operating System**

ZOHO SETU Project #10 | Software Domain | Version 1.0

🍋 *Clean, Minimal, Zesty*

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [System Architecture](#system-architecture)
3. [Build Environment Setup](#build-environment-setup)
4. [Building NIMBU OS](#building-nimbu-os)
5. [Running and Testing](#running-and-testing)
6. [Project Structure](#project-structure)
7. [Component Documentation](#component-documentation)
8. [Implementation Details](#implementation-details)
9. [Shell Commands](#shell-commands)
10. [Troubleshooting](#troubleshooting)

---

## Project Overview

NIMBU OS is a minimal, from-scratch Linux-based operating system developed as part of the ZOHO SETU Project-Based Learning initiative. The project demonstrates understanding of core operating system concepts including:

- **Kernel Architecture**: Monolithic kernel in x86 32-bit architecture
- **Memory Management**: Physical and virtual memory with paging
- **Process Management**: Round-robin preemptive scheduler
- **Interrupt Handling**: IDT-based interrupt dispatch
- **Device Drivers**: VGA display, PS/2 keyboard, serial port
- **Shell**: Minimal command-line interface

### Name Significance

**NIMBU** (Hindi for "lemon") 🍋 reflects the project philosophy:
- **Clean**: Minimal codebase, clear separation of concerns
- **Zesty**: Refreshingly different approach to OS learning
- **Minimal**: Only essential components, no bloat

---

## System Architecture

### High-Level Layers

```
Layer 5 │ Shell (nimbu-shell)
        │ Command-line interface
        │
Layer 4 │ Device Drivers
        │ VGA, Keyboard, Serial, Timer
        │
Layer 3 │ Interrupt Handling
        │ IDT, ISRs, IRQs
        │
Layer 2 │ Process Management
        │ Scheduler, PCBs, Context Switching
        │
Layer 1 │ Memory Management
        │ PMM, VMM, Heap, Paging
        │
Layer 0 │ Bootloader & CPU
        │ GDT, Protected Mode, Boot Menu
```

### Memory Layout (32-bit x86)

```
0xC0400000  ┌─────────────────────┐
            │  Kernel Heap (8 MB) │
            ├─────────────────────┤
0xC0100000  │  Kernel Code/Data   │
            ├─────────────────────┤
0xC0000000  │  Kernel Base (3 GB) │
            │  (Higher-half)      │
    ┌───────────────────────────────┐
    │  User Space (0 - 3 GB)        │
    │  (Not fully implemented)      │
0x00100000  ├─────────────────────┤
            │  Kernel Loaded Here │
0x00000000  │  Real Mode IVT      │
            └─────────────────────┘
```

### Interrupt Layout

```
Vector  │ Type              │ Handler
────────┼───────────────────┼─────────────────
0-31    │ CPU Exceptions    │ ISR (int 0-31)
32-47   │ Hardware IRQs      │ IRQ (int 32-47)
48-127  │ Reserved          │ Available
128     │ Syscall           │ int 0x80
129-255 │ Available         │ Custom handlers
```

---

## Build Environment Setup

### System Requirements

- **Host OS**: Ubuntu 22.04 LTS (or compatible Linux)
- **Disk Space**: ~500 MB for toolchain and build artifacts
- **Memory**: 4 GB RAM minimum

### Step 1: Install Required Packages

```bash
# Install GCC cross-compiler toolchain
sudo apt update
sudo apt install build-essential binutils-multiarch bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

# Install NASM assembler
sudo apt install nasm

# Install QEMU emulator for testing
sudo apt install qemu-system-x86

# Install GDB for debugging
sudo apt install gdb

# Install additional tools
sudo apt install tree git make
```

### Step 2: Build i686-elf Cross-Compiler

If precompiled cross-compiler is not available:

```bash
# Create a directory for cross-compiler
mkdir -p ~/osdev/cross-compiler
cd ~/osdev/cross-compiler

# Download GCC source (version 12.x recommended)
wget https://ftpmirror.gnu.org/gcc/gcc-12.2.0/gcc-12.2.0.tar.xz
tar xf gcc-12.2.0.tar.xz

# Download binutils source
wget https://ftpmirror.gnu.org/binutils/binutils-2.39.tar.xz
tar xf binutils-2.39.tar.xz

# Create build directories
mkdir build-binutils build-gcc

# Build binutils
cd build-binutils
../binutils-2.39/configure --target=i686-elf --prefix="$HOME/osdev/cross-compiler/install" --with-sysroot --disable-nls --disable-werror
make
make install

# Build GCC
cd ../build-gcc
../gcc-12.2.0/configure --target=i686-elf --prefix="$HOME/osdev/cross-compiler/install" --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

# Add to PATH
export PATH="$HOME/osdev/cross-compiler/install/bin:$PATH"
```

### Step 3: Verify Installation

```bash
# Verify compiler
i686-elf-gcc --version
i686-elf-ld --version

# Verify assembler
nasm -version

# Verify QEMU
qemu-system-i386 --version

# Verify GDB
gdb --version
```

---

## Building NIMBU OS

### Directory Structure

```
nimbu-os/
├── boot/                    # Bootloader
│   ├── boot.asm            # Boot entry point
│   └── boot_menu.asm       # Boot menu (optional)
├── src/                     # Source files
│   ├── kernel/
│   │   ├── arch/x86/       # x86-specific code
│   │   │   ├── gdt.c       # Global Descriptor Table
│   │   │   └── idt.c       # Interrupt Descriptor Table
│   │   ├── mm/             # Memory management
│   │   │   ├── pmm.c       # Physical Memory Manager
│   │   │   ├── vmm.c       # Virtual Memory Manager
│   │   │   └── heap.c      # Kernel Heap
│   │   ├── proc/           # Process management
│   │   │   ├── process.c   # Process Control Blocks
│   │   │   └── scheduler.c # Process scheduler
│   │   ├── drivers/        # Device drivers
│   │   │   ├── vga.c       # VGA display driver
│   │   │   ├── kbd.c       # Keyboard driver
│   │   │   └── serial.c    # Serial port driver
│   │   ├── shell/          # Shell
│   │   │   └── shell.c     # Command interface
│   │   └── kernel.c        # Kernel main entry
├── include/                 # Header files
│   ├── types.h             # Type definitions
│   ├── mem.h               # Memory interfaces
│   ├── interrupt.h         # Interrupt interfaces
│   ├── process.h           # Process interfaces
│   ├── driver.h            # Driver interfaces
│   ├── shell.h             # Shell interfaces
│   └── gdt.h               # GDT interfaces
├── scripts/                 # Build scripts
│   └── build.sh            # Build helper
├── Makefile                 # Build configuration
├── linker.ld               # Linker script
├── README.md               # This file
└── .gitignore              # Git ignore rules
```

### Building

```bash
# Clone or navigate to project directory
cd ~/nimbu-os

# Build the kernel
make clean
make

# Output: build/bin/nimbu.bin (binary) and build/bin/nimbu.elf (ELF)
```

### Build Process

The Makefile performs:

1. **Assemble bootloader** → boot.o
2. **Compile kernel C sources** → .o files
   - Uses `-ffreestanding` (no standard library)
   - Uses `-nostdlib` (no runtime library)
3. **Link object files** → nimbu.elf
4. **Convert to binary** → nimbu.bin

### Build Time

- Clean build: ~10-30 seconds
- Incremental build: ~2-5 seconds

---

## Running and Testing

### Run in QEMU

```bash
# Simple run
make run

# With serial output
make run 2>&1 | tee serial.log

# Run with 256 MB RAM
qemu-system-i386 -m 256 -kernel build/bin/nimbu.elf
```

### Debug with GDB

```bash
# Terminal 1: Start QEMU with debugging
make debug

# Terminal 2: Connect GDB (in another terminal)
gdb build/bin/nimbu.elf
(gdb) target remote :1234
(gdb) break kmain
(gdb) continue
(gdb) step
(gdb) next
(gdb) backtrace
(gdb) info registers
(gdb) x/10i $pc
```

### Expected Output

```
=====================================
  NIMBU OS - Minimal Linux Kernel
  Version: 1.0
  Build: 2026-01-15
  Architecture: x86 (i686)
=====================================

[OK] Initializing GDT...
[OK] Initializing IDT...
[OK] Initializing PMM (128 MB)...
[OK] Initializing VMM with paging...
[OK] Initializing kernel heap...
[OK] Initializing process management...
[OK] Initializing scheduler (100 Hz)...
[OK] Initializing keyboard driver...
[OK] Initializing serial driver...
[OK] Initializing shell...

Enabling interrupts and starting shell...

>>> NIMBU Shell initialized
>>> Type 'help' for available commands

nimbu@localhost:~$ 
```

---

## Project Structure

### Boot Stage

- **File**: `boot/boot.asm`
- **Function**: Initial CPU mode switch and stack setup
- **Entry**: `_start` label (called by bootloader)
- **Output**: Jumps to `kmain()`

### Kernel Initialization

- **File**: `src/kernel/kernel.c`
- **Function**: `kmain()` - main entry point
- **Initializes**: All subsystems in order
- **Sequence**:
  1. VGA driver (for console output)
  2. GDT (segmentation)
  3. IDT (interrupts)
  4. PMM (physical memory)
  5. VMM (virtual memory, enable paging)
  6. Heap (dynamic allocation)
  7. Process management
  8. Scheduler
  9. Device drivers
  10. Shell

### Memory Management

#### Physical Memory Manager (PMM)

- **File**: `src/kernel/mm/pmm.c`
- **Algorithm**: Bitmap-based page frame allocator
- **Page Size**: 4 KB
- **Total Memory**: 128 MB (default)
- **Tracking**: 1 bit per page in bitmap
- **Operations**: `pmm_alloc()`, `pmm_free()`

#### Virtual Memory Manager (VMM)

- **File**: `src/kernel/mm/vmm.c`
- **Memory Model**: Higher-half kernel (0xC0000000)
- **Paging**: 32-bit paging (2-level page directory)
- **Page Directory**: 1024 entries (4 MB coverage each)
- **Page Table**: 1024 entries (4 KB coverage each)
- **Operations**: `vmm_map()`, `vmm_unmap()`, `vmm_get_phys()`

#### Kernel Heap

- **File**: `src/kernel/mm/heap.c`
- **Algorithm**: Simple first-fit allocator
- **Size**: 8 MB
- **Header Format**: Magic, size, used flag
- **Minimum Allocation**: 32 bytes
- **Operations**: `kmalloc()`, `kfree()`

### Process Management

#### Process Control Block (PCB)

- **File**: `src/kernel/proc/process.c`
- **Structure**: Contains PID, state, registers, memory info
- **States**: READY, RUNNING, BLOCKED, ZOMBIE
- **Max Processes**: 256
- **Stack Size**: One page (4 KB) per process

#### Scheduler

- **File**: `src/kernel/proc/scheduler.c`
- **Algorithm**: Round-robin preemptive
- **Time Slice**: 10 ticks (100 ms at 100 Hz)
- **Trigger**: PIT timer interrupt (IRQ0)
- **Context Switching**: Via ISR handler

### Interrupt Handling

#### IDT Setup

- **File**: `src/kernel/arch/x86/idt.c`
- **Entry Count**: 256 vectors
- **ISR Handlers**: 0-31 (CPU exceptions)
- **IRQ Handlers**: 32-47 (hardware interrupts)
- **Syscall Vector**: 128 (int 0x80)

#### PIC Initialization

- **Controller**: 8259 PIC (Programmable Interrupt Controller)
- **Master**: I/O ports 0x20-0x21
- **Slave**: I/O ports 0xA0-0xA1
- **Remapping**: IRQs mapped to vectors 32-47

### Device Drivers

#### VGA Display

- **File**: `src/kernel/drivers/vga.c`
- **Mode**: 80x25 text mode
- **Memory**: 0xB8000 (memory-mapped)
- **Color**: 4-bit attribute (foreground + background)
- **Features**: Character output, cursor control, scrolling

#### PS/2 Keyboard

- **File**: `src/kernel/drivers/kbd.c`
- **Port**: 0x60 (data), 0x64 (status)
- **IRQ**: IRQ1 (vector 33)
- **Encoding**: Scancode to ASCII translation
- **Buffer**: 256-character circular buffer

#### Serial Port (UART)

- **File**: `src/kernel/drivers/serial.c`
- **Port**: COM1 (0x3F8)
- **Baud**: 115200
- **Format**: 8N1 (8 bits, no parity, 1 stop bit)
- **Use**: Debug output, kernel logging

### Shell

- **File**: `src/kernel/shell/shell.c`
- **Commands**: help, echo, clear, mem, ps, uname, reboot, ls, info
- **Prompt**: `nimbu@localhost:~$`
- **Input**: Via keyboard driver
- **Output**: Via VGA driver

---

## Component Documentation

### File Headers

Every source file includes comprehensive documentation:

```c
/**
 * @file filename.c
 * @brief Brief description of module
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Detailed description of what this module does,
 * its role in the system, and important design decisions.
 */
```

### Function Documentation

All functions are documented with Doxygen-style comments:

```c
/**
 * @brief Short description
 * 
 * Longer description explaining the function,
 * its purpose, and expected behavior.
 * 
 * @param param1 Description of parameter
 * @param param2 Description of parameter
 * @return Description of return value
 * @note Important notes about usage
 */
int function_name(int param1, int param2);
```

---

## Implementation Details

### Global Descriptor Table (GDT)

The GDT defines memory segments for the CPU:

- **Entry 0**: NULL (required)
- **Entry 1**: Kernel Code (Ring 0, executable)
- **Entry 2**: Kernel Data (Ring 0, writable)
- **Entry 3**: User Code (Ring 3, executable)
- **Entry 4**: User Data (Ring 3, writable)
- **Entry 5**: TSS (Task State Segment, minimal)

All segments use flat 32-bit model (base 0, limit 4GB).

### Interrupt Descriptor Table (IDT)

The IDT maps CPU interrupt vectors to handler functions:

```c
typedef struct {
    uint16_t offset_low;        /* Handler low 16 bits */
    uint16_t selector;          /* Code segment selector */
    uint8_t reserved;
    uint8_t flags;              /* Type, privilege */
    uint16_t offset_high;       /* Handler high 16 bits */
} idt_entry_t;
```

### Protected Mode Transition

```asm
; 1. Load GDT pointer
lgdt [gdt_ptr]

; 2. Set Protected Mode bit
mov eax, cr0
or eax, 1
mov cr0, eax

; 3. Long jump to flush instruction queue
jmp 0x08:flush_pipeline
```

### Paging Setup

```c
/* 1. Fill page tables with page entries */
page_table[index] = physical_addr | (PAGE_PRESENT | PAGE_WRITE);

/* 2. Fill page directory with page table entries */
page_dir[index] = page_table_phys | (PAGE_PRESENT | PAGE_WRITE);

/* 3. Load page directory into CR3 */
mov cr3, page_dir_phys

/* 4. Enable paging by setting CR0.PG bit */
mov eax, cr0
or eax, 0x80000000
mov cr0, eax
```

### Context Switching

The scheduler performs context switching in the PIT timer interrupt handler:

```c
/* 1. Save current process state */
current->context = registers_from_interrupt;

/* 2. Find next READY process */
next = scheduler_find_next_ready_process();

/* 3. Load new process state */
return next->context;

/* 4. CPU restores registers and returns from interrupt */
iret;
```

---

## Shell Commands

### help
Display available commands and descriptions.

```
nimbu@localhost:~$ help
Available commands:
  help - Display available commands
  echo - Echo arguments to screen
  clear - Clear screen
  mem - Display memory statistics
  ps - List processes
  uname - Display system information
  reboot - Reboot system
```

### echo
Echo arguments to the screen.

```
nimbu@localhost:~$ echo Hello World
Hello World
```

### clear
Clear the screen.

```
nimbu@localhost:~$ clear
[screen clears]
```

### mem
Display memory statistics.

```
nimbu@localhost:~$ mem
Memory Status:
  Total RAM    : 128 MB
  Used (Kernel): ~4.2 MB
  Free         : ~123.8 MB
```

### ps
List running processes.

```
nimbu@localhost:~$ ps
Processes:
  PID  State
  ---  -----
    0  RUNNING
```

### uname
Display system information.

```
nimbu@localhost:~$ uname
NIMBU OS v1.0 | x86 | Kernel Build: 2026-01-15
```

### reboot
Reboot the system.

```
nimbu@localhost:~$ reboot
Rebooting system...
```

### info
Display system banner.

```
nimbu@localhost:~$ info
  🍋 NIMBU OS - Minimal Linux Kernel 🍋
  Version 1.0 | ZOHO SETU Project #10
  Clean, Minimal, Zesty
```

---

## Troubleshooting

### Build Errors

**Error**: `i686-elf-gcc: command not found`

- **Cause**: Cross-compiler not installed or not in PATH
- **Solution**: 
  ```bash
  export PATH="$HOME/osdev/cross-compiler/install/bin:$PATH"
  ```

**Error**: `nasm: command not found`

- **Cause**: NASM assembler not installed
- **Solution**: 
  ```bash
  sudo apt install nasm
  ```

**Error**: `linker.ld: No such file`

- **Cause**: Not in project root directory
- **Solution**: 
  ```bash
  cd ~/nimbu-os  # Ensure you're in the right directory
  ```

### Runtime Errors

**Issue**: Black screen in QEMU

- **Cause**: VGA driver not initialized or crash during boot
- **Solution**: 
  - Check kernel logs: `make run 2>&1 | tee boot.log`
  - Enable serial output: `make run | grep -i error`

**Issue**: Kernel panic during initialization

- **Cause**: Memory management or IDT setup failed
- **Solution**: 
  - Check initialization order in `kernel.c`
  - Debug with GDB: `make debug`

**Issue**: Keyboard not responding

- **Cause**: Keyboard interrupt handler not registered
- **Solution**: 
  - Ensure `kbd_init()` is called in `kmain()`
  - Check PIC is configured correctly

### Debugging

**Using GDB**

```bash
# Terminal 1
make debug

# Terminal 2
gdb build/bin/nimbu.elf

(gdb) target remote :1234
(gdb) break kmain
(gdb) continue
(gdb) info registers
(gdb) x/10i $eip         # Display instructions at EIP
(gdb) print/x $cr0       # Print CR0 register (hex)
(gdb) si                 # Step instruction
(gdb) ni                 # Step over
```

**Using QEMU Monitor**

```bash
# In QEMU window, press Ctrl+Alt+2 to enter monitor
(qemu) info registers
(qemu) info mem
(qemu) x/10i 0x100000
```

---

## Future Enhancements

### High Priority

1. **VFS Layer** - Virtual filesystem abstraction
2. **EXT2 Driver** - Read-only filesystem support
3. **ELF Loader** - Load user-space programs
4. **User Space** - Ring 3 privilege separation

### Medium Priority

1. **Network Stack** - TCP/IP implementation
2. **System Calls** - Expanded syscall interface
3. **Signal Handling** - Process signals

### Low Priority

1. **GUI** - Simple framebuffer-based display
2. **SMP** - Multi-processor support
3. **Advanced Features** - IPC, pipes, etc.

---

## References

1. **Linux Kernel Documentation** - https://kernel.org/
2. **OSDev Wiki** - https://wiki.osdev.org/
3. **Intel x86 Manual** - Vol. 3 (System Programming)
4. **The Little Book About OS Development** - https://littleosbook.github.io/
5. **NASM Documentation** - https://nasm.us/doc/
6. **QEMU Documentation** - https://www.qemu.org/docs/master/

---

## Authors

**Someshwar Ravindra Mankar**  
GCOE Amravati | 3rd Year CSE  
ZOHO SETU Project #10

---

## Declaration

We hereby declare that the project NIMBU OS (Project No. 10) submitted to ZOHO SETU is our original work carried out as part of the SETU project-based internship initiative by Zoho Nagpur. All references and resources used have been duly acknowledged. The implementation, documentation, and analysis presented in this project are our own.

---

## License

This project is created for educational purposes as part of the ZOHO SETU program. 

🍋 **NIMBU OS — Clean, Minimal, Zesty** 🍋

---

## Quick Start

```bash
# 1. Install toolchain
sudo apt install gcc binutils build-essential nasm qemu-system-x86 gdb

# 2. Get i686-elf-gcc (if not available as package)
# (See Build Environment Setup section)

# 3. Clone/navigate to project
cd ~/nimbu-os

# 4. Build
make clean
make

# 5. Run
make run

# 6. Type commands
help
mem
ps
uname
reboot
```

Enjoy building OS kernels! 🚀
# NIMBU-OS-

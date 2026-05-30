# NIMBU OS - Visual Architecture Reference

**Diagrams, flowcharts, and visual guides for system architecture**

---

## System Layers

```
┌─────────────────────────────────────────────┐
│                   SHELL                     │  User Interface
│     (Command-line, I/O, user commands)      │
└───────────────────┬─────────────────────────┘
                    │
┌───────────────────▼─────────────────────────┐
│              DEVICE DRIVERS                 │  I/O Hardware
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │   VGA    │  │ Keyboard │  │ Serial   │  │
│  └──────────┘  └──────────┘  └──────────┘  │
└───────────────────┬─────────────────────────┘
                    │
┌───────────────────▼─────────────────────────┐
│          PROCESS MANAGEMENT                 │  Execution Control
│  ┌──────────┐  ┌──────────────────────────┐ │
│  │Scheduler │  │Process Table & PCBs      │ │
│  │(Round-   │  │Context Switching         │ │
│  │robin)    │  └──────────────────────────┘ │
│  └──────────┘                                │
└───────────────────┬─────────────────────────┘
                    │
┌───────────────────▼─────────────────────────┐
│          INTERRUPT HANDLING                 │  Exception/Interrupt Mgmt
│  ┌──────────┐  ┌──────────────────────────┐ │
│  │   IDT    │  │ISR, IRQ Handlers         │ │
│  │(Vectors) │  │Exception Processing      │ │
│  └──────────┘  └──────────────────────────┘ │
└───────────────────┬─────────────────────────┘
                    │
┌───────────────────▼─────────────────────────┐
│          MEMORY MANAGEMENT                  │  Virtual & Physical Memory
│  ┌─────────┐  ┌──────────┐  ┌────────────┐ │
│  │  PMM    │  │   VMM    │  │    Heap    │ │
│  │(Bitmap) │  │(Paging)  │  │(first-fit) │ │
│  └─────────┘  └──────────┘  └────────────┘ │
└───────────────────┬─────────────────────────┘
                    │
┌───────────────────▼─────────────────────────┐
│           CPU & BOOT LAYER                  │  x86 Hardware
│  ┌──────────┐  ┌──────────────────────────┐ │
│  │   GDT    │  │Protected Mode, Segments  │ │
│  │(Segments)│  │Boot, Memory Setup        │ │
│  └──────────┘  └──────────────────────────┘ │
└─────────────────────────────────────────────┘
```

---

## Boot Sequence

```
BIOS/Bootloader Loads Kernel at 0x100000
           │
           ▼
    boot.asm (_start)
    ├─ Set ESP to kernel stack + 8192
    ├─ CLI (disable interrupts)
    ├─ Clear EFLAGS
    └─ CALL kmain()
           │
           ▼
    kernel.c (kmain)
    ├─ vga_init()          ◄─ Display output first
    ├─ gdt_init()          ◄─ CPU segment setup
    ├─ idt_init()          ◄─ Exception handlers
    ├─ pmm_init()          ◄─ Physical memory
    ├─ vmm_init()          ◄─ Virtual memory + paging
    ├─ Enable interrupts (STI)
    ├─ kbd_init()          ◄─ Keyboard driver
    ├─ serial_init()       ◄─ Serial port
    ├─ timer_init()        ◄─ Scheduler timer
    ├─ proc_init()         ◄─ Process management
    ├─ scheduler_init()    ◄─ Start scheduling
    └─ shell_run()         ◄─ User interface (never returns)
           │
           ▼
    System Running
    ├─ Scheduler running at 100 Hz
    ├─ Keyboard accepting input
    ├─ Shell processing commands
    └─ Interrupt handlers active
```

---

## Memory Layout

### Virtual Address Space (32-bit x86)

```
Virtual Memory Address Space (4 GB)

0xFFFFFFFF  ┌──────────────────────────────┐
            │  Kernel Space (1 GB)         │
            │  Higher-half kernel mapping  │
0xC0400000  ├──────────────────────────────┤
            │  Kernel Heap (8 MB)          │
            │  0xC0100000 - 0xC0900000     │
0xC0100000  ├──────────────────────────────┤
            │  Kernel Code/Data            │
            │  Linked at 0xC0100000        │
0xC0000000  ├──────────────────────────────┤
            │                              │
            │                              │
            │  User Space (3 GB)           │
            │  0x00000000 - 0xBFFFFFFF     │
            │  (Not fully utilized)        │
            │                              │
            │                              │
0x00100000  ├──────────────────────────────┤
            │  Kernel (loaded here phys)   │
            │  Identity mapped also        │
0x00000000  └──────────────────────────────┘
            Real Mode IVT (1 KB)
```

### Physical Address Space (128 MB)

```
Physical Memory Layout (128 MB)

0x08000000  ┌──────────────────────────────┐
            │  Available RAM               │
            │  (allocated by PMM)          │
            │                              │
0x02000000  ├──────────────────────────────┤
            │  Kernel heap space           │
            │  (grows upward)              │
0x01000000  ├──────────────────────────────┤
            │  Kernel code/data (loaded)   │
            │  At 0x100000 (1 MB)          │
            │                              │
0x00010000  ├──────────────────────────────┤
            │  PMM Bitmap (4 KB)           │
            │  At 0x1000                   │
0x00001000  ├──────────────────────────────┤
            │  BIOS & Bootloader Reserved  │
            │  (may contain data)          │
0x00000000  └──────────────────────────────┘
```

### Page Structure Example

```
Virtual Address: 0xC0101000

Breakdown:
┌─────────────┬──────────────┬─────────────┐
│ 1100 0000   │ 0001 0000    │ 0001 0000   │
│ (Dir bits)  │ (Table bits) │ (Offset)    │
└──────┬──────┴──────┬───────┴──────┬──────┘
       │             │              │
    Bits 31-22   Bits 21-12     Bits 11-0
    (768 decimal)(16 decimal)  (4096 bytes)

Lookup:
1. Read page_dir[768]  ─── points to page table
2. Read page_table[16] ─── points to physical page at 0x??1000
3. Add offset 0x000    ─── final address 0x??1000
```

---

## Paging Structure

### Two-Level Paging

```
Virtual Address
      │
      ├─► Bits 31-22 (10 bits) ─┐
      │                        │
      │                        ▼
      │                   Page Directory [1024 entries]
      │                   (4 KB)
      │                        │ Entry [768]
      │                        ▼
      │                   Page Table [1024 entries]
      │                   (4 KB)
      │
      └─► Bits 21-12 (10 bits) ─┐
                                │
                                ▼
                              Directory[PD_IDX]
                                  │
                                  ▼ (points to)
                              Table[PT_IDX]
                                  │
      ┌─────────────────────────┘
      │
      └─► Bits 11-0 (12 bits) ──┐
                                │
                                ▼
                            Physical Page (4 KB)
                            Offset within page
```

### Page Table Entry (32-bit)

```
┌──────────────────────────────────────────┐
│31                                       0│
├──────────────────────────────────┬────────┤
│   Physical Page Address (20 bits)│Flags(12)
└──────────────────────────────────┴────────┘

Flags (lower 12 bits):
Bit 0:  P (Present)
Bit 1:  RW (Writable)
Bit 2:  US (User/Supervisor)
Bit 3:  WT (Write-through)
Bit 4:  CD (Cache disable)
Bit 5:  A (Accessed)
Bit 6:  D (Dirty)
Bit 7:  PAT
Bit 8:  G (Global)
Bits 9-11: Available (unused)
```

---

## Process Scheduling

### Timeline Example

```
Process Queue: P1(10ms) → P2(10ms) → P3(10ms) → P1(10ms) ...

Time (ms)
0    10   20   30   40   50   60   70   80   90  100
|    |    |    |    |    |    |    |    |    |    |
├─P1─┼─P2─┼─P3─┼─P1─┼─P2─┼─P3─┼─P1─┼─P2─┼─P3─┼─P1─┤
|    |    |    |    |    |    |    |    |    |    |

IRQ0 (Timer) fires every 10ms:
IRQ0 → scheduler_tick() → Check if time_slice expired
       → If expired: schedule() → Context switch
```

### State Transitions

```
                ┌─────────────┐
                │   ZOMBIE    │  Process exited
                │             │
                └──────▲──────┘
                       │
                  proc_exit()
                       │
     ┌──────────────────┼──────────────────┐
     │                  │                  │
     ▼                  ▼                  ▼
┌─────────┐      ┌─────────────┐      ┌─────────┐
│  READY  │ ◄───┤   RUNNING   │ ────►│ BLOCKED │
└─────────┘      └─────────────┘      └─────────┘
     ▲                  ▲                  │
     │                  │                  │
     └──────────────────┴──────────────────┘
     schedule()      (time               (I/O
     (next process)   slice              complete)
                      expired)

Legend:
READY   - Waiting for CPU
RUNNING - Executing on CPU
BLOCKED - Waiting for I/O or resource
ZOMBIE  - Terminated, awaiting cleanup
```

---

## Interrupt Flow

### Exception Handling

```
Exception occurs
      │
      ▼
Hardware generates interrupt signal
      │
      ▼
CPU checks IDT[vector]
      │
      ▼
Load GDT descriptor (CS=0x08)
      │
      ▼
Load handler address (EIP)
      │
      ▼
Push EFLAGS, CS, EIP onto stack
      │
      ▼
CLI (disable further interrupts)
      │
      ▼
Execute ISR (Interrupt Service Routine)
      │
      ▼
Send EOI to PIC (if hardware interrupt)
      │
      ▼
Pop state from stack
      │
      ▼
IRET (Interrupt Return)
      │
      ▼
Resume execution
```

### IDT Vector Usage

```
Vector Range    Purpose              Examples
────────────    ─────────────        ────────
0-31            CPU Exceptions       Divide by zero, Page fault
32-47           Hardware IRQs         Timer (IRQ0), Keyboard (IRQ1)
48-127          Reserved/Unused
128 (0x80)      Syscalls             int 0x80 instruction
129-255         Available
```

### IRQ to Vector Mapping

```
Hardware IRQ    Vector    Handler        Source
────────────    ──────    ──────────     ──────
IRQ0            0x20      timer_handler  Programmable Interval Timer
IRQ1            0x21      kbd_handler    PS/2 Keyboard
IRQ2            0x22      -              Cascade (to secondary PIC)
IRQ3-7          0x23-27   -              Unused/Available
IRQ8-15         0x28-2F   -              Secondary PIC
```

---

## Keyboard Input Flow

```
Physical Key Press
      │
      ▼
PS/2 Keyboard Controller
      │
      ▼
Send scancode to port 0x60
      │
      ▼
Raise IRQ1 (interrupt)
      │
      ▼
CPU jumps to IRQ1 handler
      │
      ▼
Read scancode from 0x60
      │
      ▼
Check high bit:
  0 = Key pressed
  1 = Key released
      │
      ▼
Translate scancode to ASCII (using scancode_map[])
      │
      ▼
Add to keyboard buffer (circular)
      │
      ▼
Send EOI to PIC
      │
      ▼
IRET (return from interrupt)
      │
      ▼
Shell reads from kbd_get_key()
      │
      ▼
Display character and process
```

---

## VGA Display Output

### Memory Layout

```
VGA Text Buffer (memory-mapped at 0xB8000)

0xB8000
  ┌──────────────────────────────────┐
  │ Row 0: 80 characters (160 bytes) │
  │ [Char|Attr][Char|Attr]...[C|A]   │
  ├──────────────────────────────────┤
  │ Row 1: 80 characters (160 bytes) │
  ├──────────────────────────────────┤
  │ ...                              │
  ├──────────────────────────────────┤
  │ Row 24: 80 characters (160 bytes)│
0xB8FA0
  └──────────────────────────────────┘

Total: 25 rows × 80 cols × 2 bytes = 4,000 bytes
```

### Character Layout

```
VGA Character Entry (2 bytes):

Byte 0: ASCII Character
┌──────────────────┐
│   0-127, 128-255│
└──────────────────┘

Byte 1: Attribute
┌──────────────────────────────────┐
│ Bits 7-4: Background color (8 colors)
│ Bits 3-0: Foreground color (16 colors)
└──────────────────────────────────┘

Color Codes (0-7 for background, 0-15 for foreground):
0 = Black      8 = Dark Gray
1 = Blue       9 = Light Blue
2 = Green      A = Light Green
3 = Cyan       B = Light Cyan
4 = Red        C = Light Red
5 = Magenta    D = Light Magenta
6 = Brown      E = Yellow
7 = Light Gray F = White
```

### Output Example

```
Writing 'H' with green on black:
┌─────────────────────────────────┐
│ Byte 0: 0x48 (ASCII 'H')       │
│ Byte 1: 0x02 (Green on black)  │
│         └─ FG: 0x2 (Green)    │
│         └─ BG: 0x0 (Black)    │
└─────────────────────────────────┘

In memory at offset 0:
0xB8000: 0x48 0x02 [next char...]
```

---

## Process Memory Layout

```
User Process Memory (example):

0x10000         ┌────────────────────┐
                │  User Heap         │
                │  (malloc/free)     │
                ├────────────────────┤
                │  BSS (uninitialized)
                ├────────────────────┤
                │  Data (initialized)│
0x08100000      ├────────────────────┤
                │  Code (.text)      │
                ├────────────────────┤
0x08000000      │  Kernel (unmapped) │
                │  (higher-half)     │
                ├────────────────────┤
0x00000000      │  User Stack        │
                │  (grows down)      │
                └────────────────────┘
```

---

## Context Switching

### On Timer Interrupt

```
Timer fires (IRQ0 every 10ms)
      │
      ▼
Save current process state to stack:
  PUSHA      (EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP)
  PUSH DS, ES, FS, GS
      │
      ▼
Call C handler: scheduler_tick()
      │
      ▼
Check if time_slice_remaining == 0
  YES: Call schedule()
       ├─ Save current PCB context
       ├─ Find next READY process
       ├─ Load its CR3 (page directory)
       ├─ Load its registers from stack
       └─ Set current_process = next
      │
      ▼
Send EOI to PIC
      │
      ▼
Restore registers:
  POP GS, FS, ES, DS
  POPA
      │
      ▼
IRET (restore EIP, CS, EFLAGS)
      │
      ▼
Continue in new process
```

### Register Save Format (on stack)

```
Higher addresses
     ▲
     │ ESP at interrupt
     │
     ├─ EFLAGS  (pushed by CPU)
     ├─ CS      (pushed by CPU)
     ├─ EIP     (pushed by CPU)
     ├─ EBP
     ├─ ESP (old)
     ├─ EDI
     ├─ ESI
     ├─ EDX
     ├─ ECX
     ├─ EBX
     ├─ EAX
     ├─ GS
     ├─ FS
     ├─ ES
     ├─ DS      (pushed by ISR)
     │
     ▼ ESP after PUSH instructions
Lower addresses
```

---

## System Call Flow

```
User Process
      │
      ├─ Prepare arguments (EAX, EBX, ECX, EDX)
      ├─ Call: int 0x80
      │
      ▼
CPU generates interrupt 128
      │
      ▼
IDT[128] → syscall_handler
      │
      ▼
Load syscall number from EAX
      │
      ▼
Switch on syscall number:
  Case 1 (READ):   syscall_read()
  Case 2 (WRITE):  syscall_write()
  Case 3 (EXIT):   syscall_exit()
  etc.
      │
      ▼
Execute syscall (in kernel context)
      │
      ▼
Store return value in EAX
      │
      ▼
IRET (return to user process)
      │
      ▼
User process continues
```

---

## Build Process

```
Source Code
├─ boot.asm
├─ kernel.c
├─ *.c files
└─ *.h files
    │
    ├─ Compile: nasm, gcc
    │           (assembly → .o)
    │           (C → .o)
    │
    ▼
Object Files (.o)
├─ boot.o
├─ kernel.o
├─ gdt.o
├─ idt.o
├─ pmm.o
├─ vmm.o
├─ heap.o
├─ process.o
├─ scheduler.o
├─ vga.o
├─ kbd.o
├─ serial.o
└─ shell.o
    │
    ├─ Link: i686-elf-ld
    │        (linker script: linker.ld)
    │
    ▼
Executable
├─ nimbu.elf (ELF format)
└─ nimbu.bin (raw binary)
    │
    ├─ Package into ISO or
    │  load into QEMU/VM
    │
    ▼
Bootable Kernel
```

---

## Shell Command Execution

```
┌─────────────────────────────────────┐
│ Display prompt: "nimbu@localhost:~$"│
└──────────────┬──────────────────────┘
               │
               ▼
        Read characters from kbd
               │
        ┌──────┴──────┐
        │ User types  │
        │ "echo hi"   │
        └──────┬──────┘
               │
               ▼
    Line: "echo hi\n"
               │
               ▼
         Parse line
    ┌─────────────────────┐
    │ argc = 2            │
    │ argv[0] = "echo"    │
    │ argv[1] = "hi"      │
    └─────────────────────┘
               │
               ▼
    Search command table
               │
        ┌──────┴──────┐
        │ Found:       │
        │ cmd_echo()   │
        └──────┬──────┘
               │
               ▼
    Call: cmd_echo(2, ["echo", "hi"])
               │
               ▼
    Output: "hi" (to VGA display)
               │
               ▼
    Return to main loop
               │
               ▼
    Display new prompt
```

---

## GDT Entry Structure

### Descriptor Format (8 bytes)

```
Byte 0-1:  Limit (lower 16 bits)         [L15-L0]
Byte 2-3:  Base (lower 16 bits)          [B15-B0]
Byte 4:    Base (mid 8 bits)             [B23-B16]

Byte 5:    Flags & Type
           ┌────────────────────┐
           │ P  DPL  S Type    │
           │ 1  00   1 1010    │ Kernel Code
           │ 1  00   1 0010    │ Kernel Data
           │ 1  11   1 1010    │ User Code
           │ 1  11   1 0010    │ User Data
           └────────────────────┘

Byte 6:    Flags & Limit (upper 4 bits)
           ┌────────────────────┐
           │ G D/B L AVL L11-L8 │
           │ 1   1   0  0  1111 │ 4KB pages, 32-bit
           └────────────────────┘

Byte 7:    Base (upper 8 bits)           [B31-B24]
```

---

## PMM Bitmap Structure

```
Memory Region     Bitmap Bits      Status
──────────────    ──────────────   ──────────
0x00000000-0x0FFF Bits 0-3         Reserved
0x00001000-0x001F Bits 4-31        PMM Bitmap itself
0x00020000-0x0FFFF Bits 32-8191    Bootloader area
0x00100000-0x3FFFFF Bits 256-16383 Kernel (loaded)
0x00400000+ Bits 16384+          Free (allocated by PMM)

Bitmap (4 KB = 32,768 bits = 32K pages = 128 MB coverage)

Byte 0: [Bit7][Bit6][Bit5][Bit4][Bit3][Bit2][Bit1][Bit0]
        [Page7]...[Page0]  (0=free, 1=allocated)
```

---

## Quick Reference: Important Addresses

```
Real Mode & BIOS:
0x00000000  - Real Mode IVT (1 KB)
0x00000400  - BIOS Data Area

Kernel:
0x00001000  - PMM Bitmap (4 KB)
0x00100000  - Kernel loaded here (physical)
0xC0000000  - Kernel base (virtual)
0xC0100000  - Kernel heap start

Memory-Mapped I/O:
0xB8000     - VGA text buffer (80x25)

I/O Ports:
0x20        - Programmable Interrupt Controller (PIC)
0x40-0x43   - Programmable Interval Timer (PIT)
0x60        - PS/2 Keyboard data
0x64        - PS/2 Keyboard status/control
0x3F8       - Serial port (COM1)
0x3D4-0x3D5 - VGA CRT Controller registers
```

---

**Visual Reference Version**: 1.0  
**Last Updated**: May 30, 2026

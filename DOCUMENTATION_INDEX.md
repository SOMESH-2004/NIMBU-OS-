# NIMBU OS - Documentation Index & Quick Start Guide

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![GitHub](https://img.shields.io/badge/GitHub-NIMBU--OS-blue.svg)](https://github.com/yourusername/NIMBU-OS)
[![Documentation](https://img.shields.io/badge/Documentation-11000%2B%20lines-brightgreen.svg)](DOCUMENTATION.md)

**Complete documentation portal for NIMBU OS project**

---

## 📚 Documentation Overview

This project includes comprehensive documentation in multiple files. Choose the right document based on your needs:

### Main Documentation Files

| Document | Purpose | Read Time | Best For |
|----------|---------|-----------|----------|
| **[README.md](README.md)** | Project overview and quick start | 10 min | First-time visitors, getting started |
| **[DOCUMENTATION.md](DOCUMENTATION.md)** | Complete technical reference | 30 min | Understanding architecture, implementation details |
| **[API_REFERENCE.md](API_REFERENCE.md)** | Complete API documentation | 20 min | Looking up function signatures, using APIs |
| **[SOURCE_CODE_GUIDE.md](SOURCE_CODE_GUIDE.md)** | Implementation walkthroughs | 25 min | Understanding code implementation, extending OS |
| **[ARCHITECTURE.md](ARCHITECTURE.md)** | High-level architecture overview | 5 min | Quick architecture reference |
| **[SOURCE_CODE_SUMMARY.md](SOURCE_CODE_SUMMARY.md)** | Project statistics and features | 5 min | Code organization overview |
| **[VISUAL_REFERENCE.md](VISUAL_REFERENCE.md)** | Diagrams and visual guides | 10 min | Understanding system flow visually |

---

## 🚀 Quick Start Guide

### For First-Time Users

1. **Read**: Start with [README.md](README.md) (10 minutes)
   - Understand what NIMBU OS is
   - See high-level architecture
   - Learn build and run instructions

2. **Build**: Follow build instructions to compile
   ```bash
   make clean
   make all
   ```

3. **Explore**: Examine key files
   - [src/kernel/kernel.c](src/kernel/kernel.c) - Kernel initialization
   - [src/kernel/shell/shell.c](src/kernel/shell/shell.c) - User interface
   - [include/types.h](include/types.h) - Type definitions

4. **Understand**: Read [ARCHITECTURE.md](ARCHITECTURE.md) for system design

### For Developers

1. **Architecture**: Read [DOCUMENTATION.md](DOCUMENTATION.md)
   - Understand memory management
   - Learn interrupt handling
   - Study process scheduling

2. **API Reference**: Consult [API_REFERENCE.md](API_REFERENCE.md)
   - Look up function signatures
   - Find parameter descriptions
   - See usage examples

3. **Implementation**: Study [SOURCE_CODE_GUIDE.md](SOURCE_CODE_GUIDE.md)
   - Deep dive into key modules
   - Understand algorithms
   - Learn hardware interactions

4. **Extend**: Create new features following patterns in existing code

### For Students/Educators

1. **Learning Path**:
   - Week 1: [README.md](README.md) + [ARCHITECTURE.md](ARCHITECTURE.md)
   - Week 2: [DOCUMENTATION.md](DOCUMENTATION.md) - Memory Management
   - Week 3: [DOCUMENTATION.md](DOCUMENTATION.md) - Interrupt Handling
   - Week 4: [DOCUMENTATION.md](DOCUMENTATION.md) - Process Management
   - Week 5+: [SOURCE_CODE_GUIDE.md](SOURCE_CODE_GUIDE.md) - Deep dives

2. **Key Concepts**:
   - Memory management (physical, virtual, heap)
   - Interrupt handling and exception processing
   - Process scheduling and context switching
   - Device driver architecture

---

## 📖 Documentation Sections by Topic

### Getting Started
- [README.md - Project Overview](README.md#project-overview)
- [README.md - Build Instructions](README.md#building-nimbu-os)
- [README.md - Running & Testing](README.md#running-and-testing)

### Architecture & Design
- [ARCHITECTURE.md - System Layers](ARCHITECTURE.md#system-layers-bottom-to-top)
- [DOCUMENTATION.md - System Architecture](DOCUMENTATION.md#system-architecture)
- [DOCUMENTATION.md - Memory Layout](DOCUMENTATION.md#memory-layout-32-bit-x86)

### Memory Management
- [DOCUMENTATION.md - Memory Management](DOCUMENTATION.md#memory-management)
  - Physical Memory Manager (PMM)
  - Virtual Memory Manager (VMM) & Paging
  - Kernel Heap
- [SOURCE_CODE_GUIDE.md - PMM Implementation](SOURCE_CODE_GUIDE.md#physical-memory-manager-pmmc)
- [SOURCE_CODE_GUIDE.md - VMM Implementation](SOURCE_CODE_GUIDE.md#virtual-memory-manager-vmmc)
- [API_REFERENCE.md - Memory API](API_REFERENCE.md#memory-management-api)

### CPU & Interrupts
- [DOCUMENTATION.md - Interrupt Handling](DOCUMENTATION.md#interrupt-handling)
  - GDT (Global Descriptor Table)
  - IDT (Interrupt Descriptor Table)
- [SOURCE_CODE_GUIDE.md - GDT Implementation](SOURCE_CODE_GUIDE.md#gdt-implementation-gdtc)
- [SOURCE_CODE_GUIDE.md - IDT & Interrupts](SOURCE_CODE_GUIDE.md#idt--interrupts-idtc)
- [API_REFERENCE.md - Interrupt API](API_REFERENCE.md#interrupt--cpu-api)

### Process Management
- [DOCUMENTATION.md - Process Management](DOCUMENTATION.md#process-management)
  - Process Control Blocks (PCB)
  - Process States
  - Scheduler (Round-Robin)
  - Context Switching
- [SOURCE_CODE_GUIDE.md - Process Management](SOURCE_CODE_GUIDE.md#process-management-processc)
- [SOURCE_CODE_GUIDE.md - Scheduler](SOURCE_CODE_GUIDE.md#scheduler-schedulerc)
- [API_REFERENCE.md - Process API](API_REFERENCE.md#process-management-api)

### Device Drivers
- [DOCUMENTATION.md - Device Drivers](DOCUMENTATION.md#device-drivers)
  - VGA Display Driver
  - PS/2 Keyboard Driver
  - Serial Port Driver
- [SOURCE_CODE_GUIDE.md - VGA Driver](SOURCE_CODE_GUIDE.md#vga-driver-vgac)
- [SOURCE_CODE_GUIDE.md - Keyboard Driver](SOURCE_CODE_GUIDE.md#keyboard-driver-kbdc)
- [API_REFERENCE.md - Driver APIs](API_REFERENCE.md#vga-display-api)

### Shell & User Interface
- [DOCUMENTATION.md - Shell Interface](DOCUMENTATION.md#shell-interface)
- [SOURCE_CODE_GUIDE.md - Shell Implementation](SOURCE_CODE_GUIDE.md#shell-implementation-shellc)
- [API_REFERENCE.md - Shell API](API_REFERENCE.md#shell-api)

### Build System
- [DOCUMENTATION.md - Build System](DOCUMENTATION.md#build-system)
- [README.md - Build Environment](README.md#build-environment-setup)

### API Reference
- [API_REFERENCE.md - Complete API](API_REFERENCE.md)
  - Memory Management API
  - Interrupt API
  - Process Management API
  - Driver APIs
  - Type Definitions
  - Macros & Constants

### Configuration
- [DOCUMENTATION.md - Configuration](DOCUMENTATION.md#configuration--customization)
- [README.md - Troubleshooting](README.md#troubleshooting)

---

## 🔍 File Structure Reference

### Documentation Files
```
├── README.md                      ← Start here
├── DOCUMENTATION.md               ← Complete technical reference
├── API_REFERENCE.md               ← Function & API reference
├── SOURCE_CODE_GUIDE.md          ← Implementation details
├── ARCHITECTURE.md               ← Architecture overview
├── SOURCE_CODE_SUMMARY.md        ← Code statistics
├── FILE_LISTING.md               ← File inventory
└── DOCUMENTATION_INDEX.md        ← This file
```

### Source Code Structure
```
src/
├── boot/
│   └── boot.asm                  ← x86 bootloader (See: SOURCE_CODE_GUIDE.md - Bootloader)
└── kernel/
    ├── kernel.c                  ← Kernel initialization (See: SOURCE_CODE_GUIDE.md - Kernel Init)
    ├── arch/x86/
    │   ├── gdt.c                 ← GDT setup (See: SOURCE_CODE_GUIDE.md - GDT)
    │   └── idt.c                 ← IDT & interrupts (See: SOURCE_CODE_GUIDE.md - IDT)
    ├── mm/
    │   ├── pmm.c                 ← Physical memory (See: SOURCE_CODE_GUIDE.md - PMM)
    │   ├── vmm.c                 ← Virtual memory (See: SOURCE_CODE_GUIDE.md - VMM)
    │   └── heap.c                ← Heap allocation (See: DOCUMENTATION.md - Kernel Heap)
    ├── proc/
    │   ├── process.c             ← Process management (See: SOURCE_CODE_GUIDE.md - Process)
    │   └── scheduler.c           ← Scheduling (See: SOURCE_CODE_GUIDE.md - Scheduler)
    ├── drivers/
    │   ├── vga.c                 ← VGA display (See: SOURCE_CODE_GUIDE.md - VGA)
    │   ├── kbd.c                 ← Keyboard input (See: SOURCE_CODE_GUIDE.md - Keyboard)
    │   └── serial.c              ← Serial port (See: DOCUMENTATION.md - Serial API)
    └── shell/
        └── shell.c               ← Shell interface (See: SOURCE_CODE_GUIDE.md - Shell)

include/
├── types.h                        ← Type definitions (See: API_REFERENCE.md - Type Definitions)
├── mem.h                          ← Memory API (See: API_REFERENCE.md - Memory API)
├── interrupt.h                    ← Interrupt API (See: API_REFERENCE.md - Interrupt API)
├── process.h                      ← Process API (See: API_REFERENCE.md - Process API)
├── driver.h                       ← Driver API (See: API_REFERENCE.md - Driver APIs)
├── shell.h                        ← Shell API (See: API_REFERENCE.md - Shell API)
└── gdt.h                          ← GDT API (See: API_REFERENCE.md - Interrupt API)

Build files:
├── Makefile                       ← Build configuration (See: DOCUMENTATION.md - Build System)
├── linker.ld                      ← Linker script
└── scripts/build.sh               ← Build helper script
```

---

## 💡 Common Tasks & Where to Find Help

### "I want to understand X"

| Task | Document | Section |
|------|----------|---------|
| Understand overall architecture | [ARCHITECTURE.md](ARCHITECTURE.md) | System Layers |
| Learn how memory works | [DOCUMENTATION.md](DOCUMENTATION.md) | Memory Management |
| Understand scheduling | [DOCUMENTATION.md](DOCUMENTATION.md) | Process Management |
| Learn about interrupts | [DOCUMENTATION.md](DOCUMENTATION.md) | Interrupt Handling |
| See implementation details | [SOURCE_CODE_GUIDE.md](SOURCE_CODE_GUIDE.md) | Any section |
| Find function signatures | [API_REFERENCE.md](API_REFERENCE.md) | Any section |

### "I want to do X"

| Task | Steps |
|------|-------|
| **Build the project** | 1. Read [README.md](README.md) - Build Instructions<br>2. Run `make clean && make all` |
| **Run in QEMU** | 1. Follow [README.md](README.md) - Running & Testing<br>2. Run `make run` |
| **Add a new system call** | 1. Read [DOCUMENTATION.md](DOCUMENTATION.md) - Adding System Call<br>2. Study [SOURCE_CODE_GUIDE.md](SOURCE_CODE_GUIDE.md) - IDT section<br>3. Modify [src/kernel/arch/x86/idt.c](src/kernel/arch/x86/idt.c) |
| **Add a shell command** | 1. Read [SOURCE_CODE_GUIDE.md](SOURCE_CODE_GUIDE.md) - Shell Implementation<br>2. Modify [src/kernel/shell/shell.c](src/kernel/shell/shell.c) |
| **Add a new driver** | 1. Read [DOCUMENTATION.md](DOCUMENTATION.md) - Adding Device Driver<br>2. Create [src/kernel/drivers/newdev.c](src/kernel/drivers/newdev.c)<br>3. Update [Makefile](Makefile) |
| **Customize configuration** | 1. Read [DOCUMENTATION.md](DOCUMENTATION.md) - Configuration<br>2. Modify header files in [include/](include/) |
| **Debug an issue** | 1. Read [README.md](README.md) - Troubleshooting<br>2. Check [DOCUMENTATION.md](DOCUMENTATION.md) - Troubleshooting<br>3. Add debug output using vga_putstr() or serial_putstr() |

---

## 📝 Documentation Conventions

### Code Examples

Code examples show proper usage:
```c
/* Initialize memory manager with 128 MB */
if (pmm_init(128 * 1024 * 1024)) {
    vga_putstr("Memory initialized\n", VGA_COLOR_GREEN);
}
```

### Important Notes

Key information is highlighted:
> **Note**: Paging must be enabled before context switching

> **Warning**: Double-free causes heap corruption

### Links

Internal references use markdown links:
- [Function signature](API_REFERENCE.md#function-name)
- [Implementation](SOURCE_CODE_GUIDE.md#section-name)
- [Source file](src/path/to/file.c)

---

## 🎓 Learning Resources

### Essential Concepts (in order)

1. **x86 Architecture Basics**
   - Protected mode, segments, selectors
   - Reference: [SOURCE_CODE_GUIDE.md - GDT Implementation](SOURCE_CODE_GUIDE.md#gdt-implementation-gdtc)

2. **Memory Management**
   - Paging, page tables, virtual memory
   - Reference: [DOCUMENTATION.md - Memory Management](DOCUMENTATION.md#memory-management)

3. **Interrupt Handling**
   - IDT, exceptions, IRQs, interrupt handlers
   - Reference: [DOCUMENTATION.md - Interrupt Handling](DOCUMENTATION.md#interrupt-handling)

4. **Process Management**
   - Process states, scheduling, context switching
   - Reference: [DOCUMENTATION.md - Process Management](DOCUMENTATION.md#process-management)

5. **Device Drivers**
   - I/O ports, memory-mapped I/O, interrupts
   - Reference: [DOCUMENTATION.md - Device Drivers](DOCUMENTATION.md#device-drivers)

### External References

- **Intel x86 Manual**: https://www.intel.com/content/dam/develop/external/us/en/documents/pdf/319973-014.pdf
- **OSDev Wiki**: https://wiki.osdev.org/
- **Linux Kernel Development**: https://www.kernel.org/
- **Operating System Concepts**: Silberschatz, Galvin, Gagne

---

## 🔧 Development Tips

### Quick Reference Commands

```bash
# Build project
make all

# Clean build artifacts
make clean

# Run in QEMU
make run

# Debug with GDB
make debug

# View project structure
tree src/ include/

# Count lines of code
wc -l src/kernel/**/*.c include/*.h
```

### Common Debugging Techniques

1. **VGA Output**: Print debug messages to screen
   ```c
   vga_putstr("Debug: variable = ", VGA_COLOR_YELLOW);
   ```

2. **Serial Output**: Output to serial port for logging
   ```c
   serial_putstr("Debug message\n");
   ```

3. **Assertions**: Verify assumptions
   ```c
   if (!condition) {
       vga_putstr("ASSERTION FAILED\n", VGA_COLOR_RED);
   }
   ```

4. **Breakpoints**: Use GDB for stepping through code
   ```bash
   make debug
   (gdb) break kernel.c:50
   (gdb) continue
   ```

### Performance Profiling

- Timer tick counter shows how much CPU time each process uses
- Use `cmd_ps()` to see process runtimes
- Adjust `SCHEDULER_TIME_SLICE` to change scheduling behavior

---

## 📊 Quick Statistics

| Metric | Value |
|--------|-------|
| **Total Files** | ~27 |
| **Source Files** | ~20 |
| **Documentation Files** | ~7 |
| **Total Lines of Code** | ~4,090 |
| **Total Lines of Docs** | ~8,000+ |
| **Supported Processes** | 256 |
| **Maximum RAM** | 128 MB |
| **Scheduler Frequency** | 100 Hz (10 ms ticks) |
| **Kernel Heap Size** | 8 MB |

---

## 🤝 Contributing

To extend NIMBU OS:

1. **Study existing code** using [SOURCE_CODE_GUIDE.md](SOURCE_CODE_GUIDE.md)
2. **Follow naming conventions** from existing code
3. **Document changes** in relevant documentation files
4. **Test thoroughly** using shell commands and debug output
5. **Update this index** if adding new documentation

---

## 📞 Support & Questions

For questions about:

- **Building**: See [README.md - Build Environment](README.md#build-environment-setup)
- **Architecture**: See [ARCHITECTURE.md](ARCHITECTURE.md)
- **APIs**: See [API_REFERENCE.md](API_REFERENCE.md)
- **Implementation**: See [SOURCE_CODE_GUIDE.md](SOURCE_CODE_GUIDE.md)
- **Troubleshooting**: See [README.md - Troubleshooting](README.md#troubleshooting)

---

## 📄 Document Versions

| File | Version | Last Updated |
|------|---------|--------------|
| README.md | 1.0 | May 30, 2026 |
| DOCUMENTATION.md | 1.0 | May 30, 2026 |
| API_REFERENCE.md | 1.0 | May 30, 2026 |
| SOURCE_CODE_GUIDE.md | 1.0 | May 30, 2026 |
| ARCHITECTURE.md | 1.0 | May 30, 2026 |
| SOURCE_CODE_SUMMARY.md | 1.0 | May 30, 2026 |
| DOCUMENTATION_INDEX.md | 1.0 | May 30, 2026 |

---

**Happy learning! 🚀**

Start with [README.md](README.md) if you're new to the project.

---

*NIMBU OS - A Minimal Linux Kernel for Learning Operating Systems*  
**Version 1.0 | May 30, 2026**

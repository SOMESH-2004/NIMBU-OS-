/**
 * @file kernel.c
 * @brief Main kernel entry point
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Kernel initialization routine. Called after bootloader completes.
 * Initializes all subsystems in order and starts the shell.
 */

#include "types.h"
#include "mem.h"
#include "interrupt.h"
#include "process.h"
#include "driver.h"
#include "shell.h"
#include "gdt.h"

/* Kernel version information */
#define KERNEL_VERSION      "1.0"
#define KERNEL_BUILD_DATE   "2026-01-15"
#define KERNEL_ARCH         "x86 (i686)"

/**
 * @brief Print kernel startup message
 */
static void print_banner(void) {
    vga_putstr("=====================================\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    vga_putstr("  NIMBU OS - Minimal Linux Kernel\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    vga_putstr("  Version: ", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    vga_putstr(KERNEL_VERSION, VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    vga_putstr("\n  Build: ", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    vga_putstr(KERNEL_BUILD_DATE, VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    vga_putstr("\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    vga_putstr("  Architecture: ", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    vga_putstr(KERNEL_ARCH, VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    vga_putstr("\n=====================================\n\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
}

/**
 * @brief Print initialization message
 * @param message Message to print
 * @param status "OK" (green) or "FAIL" (red)
 */
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

/**
 * @brief Main kernel initialization routine
 * 
 * This is the entry point after bootloader switches to protected mode.
 * All kernel subsystems are initialized here.
 * 
 * @note This function should never return. System is halted at the end.
 */
void kmain(void) {
    /* VGA driver must be initialized first for debugging output */
    vga_init();
    print_banner();

    /* Initialize GDT for proper segment handling */
    vga_putstr("Initializing GDT...", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    if (gdt_init()) {
        vga_putstr(" OK\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr(" FAIL\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        goto halt;
    }

    /* Initialize IDT and interrupt handlers */
    vga_putstr("Initializing IDT...", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    if (idt_init()) {
        vga_putstr(" OK\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr(" FAIL\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        goto halt;
    }

    /* Initialize physical memory manager (bitmap allocator) */
    vga_putstr("Initializing PMM (128 MB)...", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    if (pmm_init(128 * 1024 * 1024)) {
        vga_putstr(" OK\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr(" FAIL\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        goto halt;
    }

    /* Initialize virtual memory manager and enable paging */
    vga_putstr("Initializing VMM with paging...", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    if (vmm_init()) {
        vga_putstr(" OK\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr(" FAIL\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        goto halt;
    }

    /* Initialize kernel heap for dynamic memory allocation */
    vga_putstr("Initializing kernel heap...", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    if (heap_init(KERNEL_VIRT_BASE + 0x100000, 8 * 1024 * 1024)) {
        vga_putstr(" OK\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr(" FAIL\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        goto halt;
    }

    /* Initialize process management */
    vga_putstr("Initializing process management...", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    if (proc_init()) {
        vga_putstr(" OK\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr(" FAIL\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        goto halt;
    }

    /* Initialize scheduler */
    vga_putstr("Initializing scheduler (100 Hz)...", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    if (scheduler_init()) {
        vga_putstr(" OK\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr(" FAIL\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        goto halt;
    }

    /* Initialize device drivers */
    vga_putstr("Initializing keyboard driver...", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    if (kbd_init()) {
        vga_putstr(" OK\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr(" FAIL\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        goto halt;
    }

    vga_putstr("Initializing serial driver...", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    if (serial_init()) {
        vga_putstr(" OK\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr(" FAIL\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        goto halt;
    }

    /* Initialize shell */
    vga_putstr("Initializing shell...", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    if (shell_init()) {
        vga_putstr(" OK\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    } else {
        vga_putstr(" FAIL\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        goto halt;
    }

    /* Enable interrupts and start shell */
    vga_putstr("\nEnabling interrupts and starting shell...\n\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    interrupts_enable();
    
    shell_run();

halt:
    /* If we get here, something went wrong */
    vga_putstr("\n\n*** KERNEL PANIC ***\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    vga_putstr("Halting system...\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    
    /* Disable interrupts and halt */
    interrupts_disable();
    while (1) {
        __asm__("hlt");
    }
}

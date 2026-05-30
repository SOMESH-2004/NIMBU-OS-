/**
 * @file gdt.c
 * @brief Global Descriptor Table implementation (x86)
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Implements GDT initialization with kernel and user segments.
 */

#include "gdt.h"

/* GDT table - 6 entries (NULL, kernel code, kernel data, user code, user data, TSS) */
static gdt_entry_t gdt[6];
static gdt_ptr_t gdt_ptr;

/**
 * @brief Set a GDT entry
 * 
 * Creates a segment descriptor with specified properties.
 */
void gdt_set_entry(uint32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    if (index >= 6) return;
    
    gdt[index].base_low = (base & 0xFFFF);
    gdt[index].base_mid = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;
    
    gdt[index].limit_low = (limit & 0xFFFF);
    gdt[index].limit_high = ((limit >> 16) & 0x0F) | ((flags & 0xF0) << 4);
    
    gdt[index].access = access;
}

/**
 * @brief External ASM function to flush GDT
 */
extern void gdt_flush_asm(uint32_t gdt_ptr_addr);

/**
 * @brief Initialize GDT
 */
int gdt_init(void) {
    /* GDT pointer */
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint32_t)&gdt;
    
    /* NULL descriptor */
    gdt_set_entry(0, 0, 0, 0, 0);
    
    /* Kernel code segment - Ring 0, executable, readable */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 
                  SEG_ACCESS_PRESENT | SEG_ACCESS_PRIVILEGE_0 | SEG_ACCESS_CODE | SEG_ACCESS_READABLE,
                  SEG_FLAG_GRANULARITY | SEG_FLAG_32BIT);
    
    /* Kernel data segment - Ring 0, writable */
    gdt_set_entry(2, 0, 0xFFFFFFFF,
                  SEG_ACCESS_PRESENT | SEG_ACCESS_PRIVILEGE_0 | SEG_ACCESS_DATA | SEG_ACCESS_WRITABLE,
                  SEG_FLAG_GRANULARITY | SEG_FLAG_32BIT);
    
    /* User code segment - Ring 3, executable, readable */
    gdt_set_entry(3, 0, 0xFFFFFFFF,
                  SEG_ACCESS_PRESENT | SEG_ACCESS_PRIVILEGE_3 | SEG_ACCESS_CODE | SEG_ACCESS_READABLE,
                  SEG_FLAG_GRANULARITY | SEG_FLAG_32BIT);
    
    /* User data segment - Ring 3, writable */
    gdt_set_entry(4, 0, 0xFFFFFFFF,
                  SEG_ACCESS_PRESENT | SEG_ACCESS_PRIVILEGE_3 | SEG_ACCESS_DATA | SEG_ACCESS_WRITABLE,
                  SEG_FLAG_GRANULARITY | SEG_FLAG_32BIT);
    
    /* TSS segment (minimal, not fully implemented) */
    gdt_set_entry(5, 0, 0xFFFFFFFF,
                  SEG_ACCESS_PRESENT | SEG_ACCESS_PRIVILEGE_0,
                  SEG_FLAG_GRANULARITY | SEG_FLAG_32BIT);
    
    /* Flush GDT */
    gdt_flush_asm((uint32_t)&gdt_ptr);
    
    return 1;
}

/**
 * @brief ASM stub for gdt_flush_asm
 * 
 * This needs to be implemented in assembly to:
 * 1. Load GDTR with gdt_ptr
 * 2. Perform long jump to reload CS
 * 3. Reload all other segment registers
 */
__asm__(
    ".globl gdt_flush_asm\n"
    "gdt_flush_asm:\n"
    "    mov 4(%esp), %eax\n"      /* Load gdt_ptr address */
    "    lgdt (%eax)\n"             /* Load GDT */
    "    ljmp $0x08, $.flush\n"     /* Long jump to reload CS */
    ".flush:\n"
    "    mov $0x10, %eax\n"         /* Kernel data segment */
    "    mov %eax, %ds\n"
    "    mov %eax, %es\n"
    "    mov %eax, %fs\n"
    "    mov %eax, %gs\n"
    "    mov %eax, %ss\n"
    "    ret\n"
);

/**
 * @file gdt.h
 * @brief Global Descriptor Table interface (x86)
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Defines structures and functions for x86 GDT management.
 * The GDT is used to define memory segments in protected mode.
 */

#ifndef GDT_H
#define GDT_H

#include "types.h"

/* GDT segment selectors */
#define GDT_SELECTOR_NULL       0x00
#define GDT_SELECTOR_KERNEL_CODE 0x08
#define GDT_SELECTOR_KERNEL_DATA 0x10
#define GDT_SELECTOR_USER_CODE  0x18 | 0x3  /* Ring 3 */
#define GDT_SELECTOR_USER_DATA  0x20 | 0x3  /* Ring 3 */
#define GDT_SELECTOR_TSS        0x28

/* Access byte flags */
#define SEG_ACCESS_PRESENT      0x80
#define SEG_ACCESS_PRIVILEGE_0  0x00  /* Ring 0 - kernel */
#define SEG_ACCESS_PRIVILEGE_3  0x60  /* Ring 3 - user */
#define SEG_ACCESS_CODE         0x08
#define SEG_ACCESS_DATA         0x00
#define SEG_ACCESS_READABLE     0x02  /* For code segments */
#define SEG_ACCESS_WRITABLE     0x02  /* For data segments */
#define SEG_ACCESS_ACCESSED     0x01

/* Flags (upper nibble of limit + flags byte) */
#define SEG_FLAG_GRANULARITY    0x08  /* 1 = 4KB granularity */
#define SEG_FLAG_32BIT          0x04  /* 1 = 32-bit operations */
#define SEG_FLAG_64BIT          0x00  /* 0 = not 64-bit */
#define SEG_FLAG_AVL            0x01

/**
 * @brief GDT descriptor entry
 * 
 * 8-byte descriptor defining a memory segment in protected mode.
 * Contains base address, limit, access byte, and flags.
 */
typedef struct {
    uint16_t limit_low;         /* Segment limit (lower 16 bits) */
    uint16_t base_low;          /* Base address (lower 16 bits) */
    uint8_t base_mid;           /* Base address (next 8 bits) */
    uint8_t access;             /* Access byte */
    uint8_t limit_high;         /* Limit (upper 4 bits) + flags (upper 4 bits) */
    uint8_t base_high;          /* Base address (upper 8 bits) */
} __attribute__((packed)) gdt_entry_t;

/**
 * @brief GDT pointer structure
 * 
 * Passed to LGDT instruction. Contains GDT size and address.
 */
typedef struct {
    uint16_t limit;             /* Size of GDT - 1 */
    uint32_t base;              /* Linear address of GDT */
} __attribute__((packed)) gdt_ptr_t;

/**
 * @brief Initialize Global Descriptor Table
 * 
 * Sets up GDT with kernel code, kernel data, user code, and user data segments.
 * All segments use flat 32-bit memory model starting at address 0.
 * 
 * @return 0 on success, -1 on failure
 */
int gdt_init(void);

/**
 * @brief Set GDT entry
 * 
 * Creates a GDT descriptor entry for a memory segment.
 * 
 * @param index GDT entry index
 * @param base Base address of segment
 * @param limit Segment limit (size - 1)
 * @param access Access byte (code/data, ring level, etc.)
 * @param flags Flags (granularity, 32-bit, etc.)
 */
void gdt_set_entry(uint32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

/**
 * @brief Flush GDT
 * 
 * Loads GDT pointer into GDTR register and reloads segment registers.
 * This is typically called after GDT is set up.
 */
void gdt_flush(void);

#endif /* GDT_H */

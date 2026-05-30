/**
 * @file vmm.c
 * @brief Virtual Memory Manager implementation
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Implements page directory and page table management with 32-bit paging.
 * Higher-half kernel layout: kernel mapped at 0xC0000000 (3GB).
 */

#include "mem.h"

/* Page directory and table structure */
#define PD_SIZE     1024  /* 1024 entries per page directory */
#define PT_SIZE     1024  /* 1024 entries per page table */
#define PAGE_ENTRY_SIZE 4 /* 4 bytes per entry */

/* Kernel page directory */
static uint32_t kernel_page_directory[PD_SIZE] __attribute__((aligned(PAGE_SIZE)));

/* Kernel page tables for higher-half kernel */
static uint32_t kernel_page_tables[4][PT_SIZE] __attribute__((aligned(PAGE_SIZE)));

/**
 * @brief Initialize VMM with higher-half kernel layout
 */
int vmm_init(void) {
    uint32_t i;
    
    /* Clear page directory */
    for (i = 0; i < PD_SIZE; i++) {
        kernel_page_directory[i] = 0;
    }
    
    /* Clear page tables */
    for (i = 0; i < 4; i++) {
        uint32_t j;
        for (j = 0; j < PT_SIZE; j++) {
            kernel_page_tables[i][j] = 0;
        }
    }
    
    /* Identity map first 16 MB (for bootloader and kernel) */
    for (i = 0; i < 0x1000; i++) {
        kernel_page_tables[0][i] = (i * PAGE_SIZE) | (PAGE_PRESENT | PAGE_WRITE);
    }
    
    /* Set up page directory entries for kernel page tables */
    kernel_page_directory[0] = ((uint32_t)&kernel_page_tables[0]) | (PAGE_PRESENT | PAGE_WRITE);
    kernel_page_directory[1] = ((uint32_t)&kernel_page_tables[1]) | (PAGE_PRESENT | PAGE_WRITE);
    kernel_page_directory[2] = ((uint32_t)&kernel_page_tables[2]) | (PAGE_PRESENT | PAGE_WRITE);
    kernel_page_directory[3] = ((uint32_t)&kernel_page_tables[3]) | (PAGE_PRESENT | PAGE_WRITE);
    
    /* Higher-half mapping: map kernel page tables at 3GB (index 768-771) */
    uint32_t higher_half_index = 768;  /* 0xC0000000 / 0x400000 */
    
    for (i = 0; i < 4; i++) {
        kernel_page_directory[higher_half_index + i] = 
            ((uint32_t)&kernel_page_tables[i]) | (PAGE_PRESENT | PAGE_WRITE);
    }
    
    /* Load page directory into CR3 */
    uint32_t cr3_value = (uint32_t)&kernel_page_directory;
    __asm__("mov %0, %%cr3" : : "r"(cr3_value));
    
    /* Enable paging by setting CR0.PG bit */
    uint32_t cr0_value;
    __asm__("mov %%cr0, %0" : "=r"(cr0_value));
    cr0_value |= 0x80000000;  /* Set PG bit */
    __asm__("mov %0, %%cr0" : : "r"(cr0_value));
    
    return 1;
}

/**
 * @brief Map virtual address to physical address
 */
int vmm_map(virt_addr_t virt, phys_addr_t phys, uint32_t flags) {
    if ((virt % PAGE_SIZE) != 0 || (phys % PAGE_SIZE) != 0) {
        return -1;  /* Not aligned */
    }
    
    /* Get page directory index */
    uint32_t pd_index = virt >> 22;
    /* Get page table index */
    uint32_t pt_index = (virt >> 12) & 0x3FF;
    
    if (pd_index >= PD_SIZE) {
        return -1;  /* Invalid address */
    }
    
    /* Get page table address from page directory */
    uint32_t pd_entry = kernel_page_directory[pd_index];
    uint32_t *page_table;
    
    if (!(pd_entry & PAGE_PRESENT)) {
        /* Need to create new page table */
        phys_addr_t new_table = pmm_alloc();
        if (new_table == 0) {
            return -1;  /* No memory */
        }
        kernel_page_directory[pd_index] = new_table | (PAGE_PRESENT | PAGE_WRITE);
        page_table = (uint32_t *)new_table;
    } else {
        page_table = (uint32_t *)(pd_entry & 0xFFFFF000);
    }
    
    /* Set page table entry */
    page_table[pt_index] = phys | flags;
    
    /* Invalidate TLB entry */
    __asm__("invlpg (%0)" : : "r"(virt));
    
    return 0;
}

/**
 * @brief Unmap virtual address
 */
int vmm_unmap(virt_addr_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;
    
    if (!(kernel_page_directory[pd_index] & PAGE_PRESENT)) {
        return -1;  /* Page table doesn't exist */
    }
    
    uint32_t *page_table = (uint32_t *)(kernel_page_directory[pd_index] & 0xFFFFF000);
    
    if (!(page_table[pt_index] & PAGE_PRESENT)) {
        return -1;  /* Page not mapped */
    }
    
    page_table[pt_index] = 0;
    
    /* Invalidate TLB entry */
    __asm__("invlpg (%0)" : : "r"(virt));
    
    return 0;
}

/**
 * @brief Get physical address for virtual address
 */
phys_addr_t vmm_get_phys(virt_addr_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;
    
    if (!(kernel_page_directory[pd_index] & PAGE_PRESENT)) {
        return 0;
    }
    
    uint32_t *page_table = (uint32_t *)(kernel_page_directory[pd_index] & 0xFFFFF000);
    
    if (!(page_table[pt_index] & PAGE_PRESENT)) {
        return 0;
    }
    
    return page_table[pt_index] & 0xFFFFF000;
}

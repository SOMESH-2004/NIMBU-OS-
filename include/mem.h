/**
 * @file mem.h
 * @brief Memory management interface
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Defines interfaces for Physical Memory Manager (PMM) and Virtual Memory Manager (VMM).
 * The kernel uses a higher-half memory layout with kernel mapped at 0xC0000000 (3GB).
 */

#ifndef MEM_H
#define MEM_H

#include "types.h"

/* Memory configuration */
#define KERNEL_VIRT_BASE    0xC0000000
#define PAGE_SIZE           4096
#define PAGE_MASK           (PAGE_SIZE - 1)
#define PAGE_SHIFT          12

/* Convert physical to virtual address and vice versa */
#define PHYS_TO_VIRT(addr)  ((addr) + KERNEL_VIRT_BASE)
#define VIRT_TO_PHYS(addr)  ((addr) - KERNEL_VIRT_BASE)

/**
 * @brief Initialize Physical Memory Manager
 * 
 * Initializes the bitmap-based page frame allocator that tracks 4KB pages
 * across all available RAM. Must be called early during kernel initialization.
 * 
 * @param total_memory Total available memory in bytes
 * @return 0 on success, -1 on failure
 */
int pmm_init(uint32_t total_memory);

/**
 * @brief Allocate a physical page frame
 * @return Physical address of allocated page, 0 on failure
 */
phys_addr_t pmm_alloc(void);

/**
 * @brief Free a physical page frame
 * @param addr Physical address of page to free
 * @return 0 on success, -1 on failure
 */
int pmm_free(phys_addr_t addr);

/**
 * @brief Get total allocated pages
 * @return Number of allocated pages
 */
uint32_t pmm_get_allocated(void);

/**
 * @brief Initialize Virtual Memory Manager
 * 
 * Sets up page directory and enables paging with higher-half kernel layout.
 * Kernel is mapped at 0xC0000000 virtual address.
 * 
 * @return 0 on success, -1 on failure
 */
int vmm_init(void);

/**
 * @brief Map virtual address to physical address
 * 
 * Creates page table entries to map a virtual address to a physical address
 * with specified flags.
 * 
 * @param virt Virtual address (must be page-aligned)
 * @param phys Physical address (must be page-aligned)
 * @param flags Page flags (PRESENT, WRITE, USER, etc.)
 * @return 0 on success, -1 on failure
 */
int vmm_map(virt_addr_t virt, phys_addr_t phys, uint32_t flags);

/**
 * @brief Unmap virtual address
 * @param virt Virtual address to unmap
 * @return 0 on success, -1 on failure
 */
int vmm_unmap(virt_addr_t virt);

/**
 * @brief Get physical address mapped to virtual address
 * @param virt Virtual address
 * @return Physical address, 0 if not mapped
 */
phys_addr_t vmm_get_phys(virt_addr_t virt);

/* Page flags */
#define PAGE_PRESENT    0x001
#define PAGE_WRITE      0x002
#define PAGE_USER       0x004
#define PAGE_PWT        0x008
#define PAGE_PCD        0x010
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040

/**
 * @brief Initialize kernel heap
 * 
 * Sets up a slab-like heap allocator for dynamic kernel memory allocation.
 * 
 * @param start Virtual address of heap start
 * @param size Heap size in bytes
 * @return 0 on success, -1 on failure
 */
int heap_init(virt_addr_t start, size_t size);

/**
 * @brief Allocate memory from kernel heap
 * 
 * Allocates a block of memory from the kernel heap. Similar to malloc().
 * 
 * @param size Number of bytes to allocate
 * @return Virtual address of allocated memory, NULL on failure
 */
void *kmalloc(size_t size);

/**
 * @brief Free memory to kernel heap
 * 
 * Frees a previously allocated block. Similar to free().
 * 
 * @param ptr Virtual address of memory to free
 * @return 0 on success, -1 on failure
 */
int kfree(void *ptr);

/**
 * @brief Get heap statistics
 * @param used Output parameter for allocated bytes
 * @param total Output parameter for total heap size
 * @return 0 on success, -1 on failure
 */
int heap_get_stats(uint32_t *used, uint32_t *total);

#endif /* MEM_H */

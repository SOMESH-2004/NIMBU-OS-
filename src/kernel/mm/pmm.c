/**
 * @file pmm.c
 * @brief Physical Memory Manager implementation
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Implements bitmap-based page frame allocator.
 * Tracks 4KB page frames across all available RAM.
 */

#include "mem.h"

/* Memory bitmap structure */
static struct {
    uint32_t *bitmap;           /* Bitmap for page tracking */
    uint32_t total_pages;       /* Total number of pages */
    uint32_t allocated_pages;   /* Number of allocated pages */
    uint32_t bitmap_size;       /* Size of bitmap in bytes */
} pmm_state = {NULL, 0, 0, 0};

/**
 * @brief Initialize PMM
 */
int pmm_init(uint32_t total_memory) {
    /* Calculate total pages (4KB each) */
    pmm_state.total_pages = total_memory / PAGE_SIZE;
    
    /* Calculate bitmap size in bytes (1 bit per page) */
    pmm_state.bitmap_size = (pmm_state.total_pages + 7) / 8;
    
    /* Place bitmap at 0x200000 (kernel reserved area) */
    pmm_state.bitmap = (uint32_t *)(KERNEL_VIRT_BASE + 0x200000);
    
    /* Initialize bitmap to 0 (all pages free) */
    uint8_t *bitmap_bytes = (uint8_t *)pmm_state.bitmap;
    for (uint32_t i = 0; i < pmm_state.bitmap_size; i++) {
        bitmap_bytes[i] = 0;
    }
    
    pmm_state.allocated_pages = 0;
    
    return 1;
}

/**
 * @brief Set bit in bitmap
 */
static void bitmap_set_bit(uint32_t bit) {
    uint32_t byte_idx = bit / 8;
    uint8_t bit_idx = bit % 8;
    ((uint8_t *)pmm_state.bitmap)[byte_idx] |= (1 << bit_idx);
}

/**
 * @brief Clear bit in bitmap
 */
static void bitmap_clear_bit(uint32_t bit) {
    uint32_t byte_idx = bit / 8;
    uint8_t bit_idx = bit % 8;
    ((uint8_t *)pmm_state.bitmap)[byte_idx] &= ~(1 << bit_idx);
}

/**
 * @brief Test bit in bitmap
 */
static int bitmap_test_bit(uint32_t bit) {
    uint32_t byte_idx = bit / 8;
    uint8_t bit_idx = bit % 8;
    return (((uint8_t *)pmm_state.bitmap)[byte_idx] >> bit_idx) & 1;
}

/**
 * @brief Find first free page
 */
static uint32_t find_free_page(void) {
    for (uint32_t i = 0; i < pmm_state.total_pages; i++) {
        if (!bitmap_test_bit(i)) {
            return i;
        }
    }
    return 0xFFFFFFFF;  /* No free pages */
}

/**
 * @brief Allocate physical page
 */
phys_addr_t pmm_alloc(void) {
    uint32_t page_idx = find_free_page();
    if (page_idx == 0xFFFFFFFF) {
        return 0;  /* No free pages */
    }
    
    bitmap_set_bit(page_idx);
    pmm_state.allocated_pages++;
    
    return page_idx * PAGE_SIZE;
}

/**
 * @brief Free physical page
 */
int pmm_free(phys_addr_t addr) {
    if (addr % PAGE_SIZE != 0) {
        return -1;  /* Not aligned */
    }
    
    uint32_t page_idx = addr / PAGE_SIZE;
    if (page_idx >= pmm_state.total_pages) {
        return -1;  /* Invalid page index */
    }
    
    if (!bitmap_test_bit(page_idx)) {
        return -1;  /* Page not allocated */
    }
    
    bitmap_clear_bit(page_idx);
    pmm_state.allocated_pages--;
    
    return 0;
}

/**
 * @brief Get allocated pages count
 */
uint32_t pmm_get_allocated(void) {
    return pmm_state.allocated_pages;
}

/**
 * @brief Get total pages
 */
uint32_t pmm_get_total(void) {
    return pmm_state.total_pages;
}

/**
 * @brief Get free pages count
 */
uint32_t pmm_get_free(void) {
    return pmm_state.total_pages - pmm_state.allocated_pages;
}

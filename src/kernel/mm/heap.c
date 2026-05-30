/**
 * @file heap.c
 * @brief Kernel heap implementation
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Implements simple slab-like heap allocator for kmalloc/kfree.
 */

#include "mem.h"

/* Heap metadata */
static struct {
    uint8_t *start;
    uint32_t size;
    uint32_t used;
} heap_state = {NULL, 0, 0};

/* Allocation header */
typedef struct {
    uint32_t magic;     /* Magic number for validation */
    uint32_t size;      /* Size of allocated block */
    uint8_t used;       /* Is block used? */
} alloc_header_t;

#define ALLOC_MAGIC     0xDEADBEEF
#define MIN_ALLOC_SIZE  32

/**
 * @brief Initialize heap
 */
int heap_init(virt_addr_t start, size_t size) {
    heap_state.start = (uint8_t *)start;
    heap_state.size = size;
    heap_state.used = 0;
    
    return 1;
}

/**
 * @brief Allocate memory
 */
void *kmalloc(size_t size) {
    if (size == 0 || size > heap_state.size) {
        return NULL;
    }
    
    /* Add space for header and align */
    size_t total_size = sizeof(alloc_header_t) + size;
    if (total_size < MIN_ALLOC_SIZE) {
        total_size = MIN_ALLOC_SIZE;
    }
    
    /* Find free block */
    uint8_t *current = heap_state.start;
    uint32_t offset = 0;
    
    while (offset < heap_state.used) {
        alloc_header_t *header = (alloc_header_t *)current;
        
        if (header->magic != ALLOC_MAGIC) {
            return NULL;  /* Heap corruption */
        }
        
        if (!header->used && header->size >= total_size) {
            /* Found free block - use it */
            header->used = 1;
            return (void *)(current + sizeof(alloc_header_t));
        }
        
        current += header->size;
        offset += header->size;
    }
    
    /* Allocate new block at end of heap */
    if (heap_state.used + total_size > heap_state.size) {
        return NULL;  /* Out of memory */
    }
    
    alloc_header_t *header = (alloc_header_t *)current;
    header->magic = ALLOC_MAGIC;
    header->size = total_size;
    header->used = 1;
    
    heap_state.used += total_size;
    
    return (void *)(current + sizeof(alloc_header_t));
}

/**
 * @brief Free memory
 */
int kfree(void *ptr) {
    if (ptr == NULL) {
        return 0;
    }
    
    alloc_header_t *header = (alloc_header_t *)ptr - 1;
    
    if (header->magic != ALLOC_MAGIC) {
        return -1;  /* Invalid pointer */
    }
    
    header->used = 0;
    
    return 0;
}

/**
 * @brief Get heap statistics
 */
int heap_get_stats(uint32_t *used, uint32_t *total) {
    if (used) *used = heap_state.used;
    if (total) *total = heap_state.size;
    return 0;
}

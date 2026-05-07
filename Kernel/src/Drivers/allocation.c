// allocation.c
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#define PAGE_SIZE 4096
#define ALIGN16(x) (((x) + 15) & ~15)


__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};


static uint64_t hhdm_offset = 0;

static uint8_t *bitmap;
static size_t bitmap_size;
static size_t total_pages;

// Heap
static uint8_t* heap_start;
static uint8_t* heap_end;
static uint8_t* heap_ptr;


static inline void* phys_to_virt(void* phys) {
    return (void*)((uint64_t)phys + hhdm_offset);
}

static inline void* virt_to_phys(void* virt) {
    return (void*)((uint64_t)virt - hhdm_offset);
}


static inline void bitmap_set(size_t i) {
    bitmap[i / 8] |= (1 << (i % 8));
}

static inline void bitmap_clear(size_t i) {
    bitmap[i / 8] &= ~(1 << (i % 8));
}

static inline bool bitmap_test(size_t i) {
    return bitmap[i / 8] & (1 << (i % 8));
}


void pmm_init(void) {
    struct limine_memmap_response *res = memmap_request.response;
    hhdm_offset = hhdm_request.response->offset;

    // Find highest address
    uint64_t highest = 0;
    for (size_t i = 0; i < res->entry_count; i++) {
        uint64_t end = res->entries[i]->base + res->entries[i]->length;
        if (end > highest)
            highest = end;
    }

    total_pages = highest / PAGE_SIZE;
    bitmap_size = total_pages / 8 + 1;

    // TEMP: place bitmap at 1MB (must be usable!)
    bitmap = phys_to_virt((void*)0x100000);

    // Mark all used
    for (size_t i = 0; i < bitmap_size; i++)
        bitmap[i] = 0xFF;

    // Mark usable as free
    for (size_t i = 0; i < res->entry_count; i++) {
        if (res->entries[i]->type == LIMINE_MEMMAP_USABLE) {
            uint64_t base = res->entries[i]->base;
            uint64_t length = res->entries[i]->length;

            for (uint64_t addr = base; addr < base + length; addr += PAGE_SIZE) {
                size_t page = addr / PAGE_SIZE;
                bitmap_clear(page);
            }
        }
    }
}

void* pmm_alloc_page(void) {
    for (size_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

void pmm_free_page(void* addr) {
    size_t page = (uint64_t)addr / PAGE_SIZE;
    bitmap_clear(page);
}

#define HEAP_PAGES 16

void heap_init(void) {
    void* first = pmm_alloc_page();
    if (!first) return;

    heap_start = phys_to_virt(first);
    heap_ptr   = heap_start;

    // Allocate contiguous pages (simple version)
    for (int i = 1; i < HEAP_PAGES; i++) {
        pmm_alloc_page();
    }

    heap_end = heap_start + HEAP_PAGES * PAGE_SIZE;
}

void* malloc(size_t size) {
    size = ALIGN16(size);

    if (heap_ptr + size > heap_end)
        return NULL;

    void* ptr = heap_ptr;
    heap_ptr += size;
    return ptr;
}

void free(void* ptr) {
    (void)ptr;
}

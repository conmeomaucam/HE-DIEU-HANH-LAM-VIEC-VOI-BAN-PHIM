#include "heap.h"
extern uint32_t _heap_start;
static uint8_t* heap_ptr = 0;
void heap_init(void) {
    heap_ptr = (uint8_t*)&_heap_start;
}
void* malloc(uint32_t size) {
    size = (size + 3) & ~3;
    void* ptr = heap_ptr;
    heap_ptr += size;
    return ptr;
}
void free(void* ptr) { }

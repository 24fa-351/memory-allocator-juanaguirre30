#include "memoryallocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>

heap_t* heap = NULL;

heap_t* heap_create(int capacity) {
    void* created_block = get_me_blocks(sizeof(heap_t) + capacity * sizeof(chunk_on_heap_t));
    heap_t* heap = (heap_t*)created_block;
    heap->data = (chunk_on_heap_t*)((char*)created_block + sizeof(heap_t));
    heap->size = 0;
    heap->capacity = capacity;

    return heap;
}

void* get_me_blocks(ssize_t how_much) {
    void* ptr = sbrk(how_much);
    if (ptr == (void*)-1) {
        fprintf(stderr, "sbrk failed\n");
        exit(1);
    }
    return ptr;
}

void heap_swap(heap_t* heap, int index1, int index2) {
    chunk_on_heap_t temp = heap->data[index1];

    heap->data[index1] = heap->data[index2];
    heap->data[index2] = temp;
}

void heap_bubble_up(heap_t* heap, int index) {
    unsigned int parent = (index - 1) / 2;

    while (index > 0 && heap->data[index].size < heap->data[parent].size) {
        heap_swap(heap, index, parent);
        index = parent;
        parent = (index - 1) / 2;
    }
}

void heap_bubble_down(heap_t* heap, int index) {
    unsigned int current = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < heap->size && heap->data[left].size < heap->data[current].size) {
        current = left;
    }

    if (right < heap->size && heap->data[right].size < heap->data[current].size) {
        current = right;
    }

    if (current != index) {
        heap_swap(heap, index, current);
        heap_bubble_down(heap, current);
    }
}

void heap_insert(heap_t* heap, int size, char* pointer) {
    if (heap->size == heap->capacity) {
        void* new_memory = get_me_blocks((heap->capacity) * sizeof(chunk_on_heap_t));
        heap->data = (chunk_on_heap_t*)((char*)heap->data + (uintptr_t)new_memory);
        heap->capacity *= 2;
    }

    heap->data[heap->size].size = size;
    heap->data[heap->size].pointer_to_start = pointer;
    heap->size++;

    heap_bubble_up(heap, heap->size - 1);
}

chunk_on_heap_t heap_remove_min(heap_t* heap) {
    if (heap->size == 0) {
        fprintf(stderr, "Heap is empty\n");
        return (chunk_on_heap_t){0, NULL}; 
    }

    chunk_on_heap_t min_chunk = heap->data[0];

    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;

    heap_bubble_down(heap, 0);
    return min_chunk;
}

void initialize_heap() {
    if (heap == NULL) {
        heap = heap_create(1024);
    }
}

void* my_malloc(size_t size) {
    initialize_heap();

    size_t new_size = size + sizeof(size_t);
    int best_fit_index = -1;
    int best_fit_size = INT_MAX; 

    for (int ix = 0; ix < heap->size; ix++) {
        if (heap->data[ix].size >= new_size && heap->data[ix].size < best_fit_size) {
            best_fit_size = heap->data[ix].size;
            best_fit_index = ix;
        }
    }

    if (best_fit_index != -1) {
        chunk_on_heap_t chunk = heap->data[best_fit_index];

        heap->data[best_fit_index].size = 0;
        heap->data[best_fit_index].pointer_to_start = NULL;

        heap->data[best_fit_index] = heap->data[heap->size - 1];
        heap->size--;
        heap_bubble_down(heap, best_fit_index);

        if (chunk.size >= new_size) {
            int leftover_size = chunk.size - new_size;
            if (leftover_size > 0) {
                char* leftover = chunk.pointer_to_start + new_size;
                heap_insert(heap, leftover_size, leftover);
            }
            *((size_t*)chunk.pointer_to_start) = size;
            return (void*)(chunk.pointer_to_start + sizeof(size_t));
        }
    }

    void* new_block = get_me_blocks(new_size);
    if (new_block == (void*)-1) {
        return NULL;
    }
    *((size_t*)new_block) = size;
    
    return (void*)((char*)new_block + sizeof(size_t));
}

void my_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    char* final_ptr = (char*)ptr - sizeof(size_t);
    size_t block_size = *((size_t*)final_ptr);

    chunk_on_heap_t freed_chunk;
    freed_chunk.pointer_to_start = final_ptr;
    freed_chunk.size = block_size + sizeof(size_t);

    heap_insert(heap, freed_chunk.size, freed_chunk.pointer_to_start);
}

void* my_realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return my_malloc(size);
    }

    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    char* real_ptr = (char*)ptr - sizeof(size_t);
    size_t current_size = *((size_t*)real_ptr);

    if (size <= current_size) {
        return ptr;
    }

    void* reallocated_block = my_malloc(size);
    if (reallocated_block == NULL) {
        return NULL;
    }

    memcpy(reallocated_block, ptr, current_size);

    my_free(ptr);

    return reallocated_block;
}

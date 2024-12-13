#include <stddef.h>
#include <unistd.h>
#include <string.h>

struct chunk_on_heap {
    int size;
    char* pointer_to_start;
};
typedef struct chunk_on_heap chunk_on_heap_t;

struct heap {
    chunk_on_heap_t* data;
    int size;
    int capacity;
};
typedef struct heap heap_t;

// min-heap functions from min-heap assignment 
heap_t* heap_create(int capacity);
void heap_free(heap_t* heap);
void heap_insert(heap_t* heap, int size, char* pointer);
chunk_on_heap_t heap_remove_min(heap_t* heap);
void heap_bubble_up(heap_t* heap, int index);
void heap_bubble_down(heap_t* heap, int index);
void initialize_heap();

// get_me_blocks function
void* get_me_blocks(ssize_t how_much);

// memory allocation functions
void* my_malloc(size_t size);
void my_free(void* ptr);
void* my_realloc(void* ptr, size_t size);

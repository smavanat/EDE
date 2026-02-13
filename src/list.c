#include "../include/list.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * Grows the given list by a factor of 2. Only used when the list is at capacity
 * @param slice the list to grow
 * @param size the size of the elements in the list in bytes
 */
void grow(list *slice, size_t size) {
    size_t new_capacity = (slice->capacity > 0) ? slice->capacity * 2 : 1; //If the capacity was 0, set it to 1, otherwise multiply by 2

    //Allocate the new data buffer and copy the data over
    void *data = malloc(new_capacity*size);
    if(slice->size > 0) {
        memcpy(data, slice->data, slice->size*size);
    }
    free(slice->data); //Get rid of the old buffer
    slice->data = data;//Swap the buffers
    slice->capacity = new_capacity; //Update the capacity
}

/**
 * Allocates a new list
 * @param arr_size the initial number of elements the list should have
 * @param t_size the size of the type the list should store
 * @return a pointer to the created list on the heap
 */
list *list_alloc(size_t arr_size, size_t t_size) {
    size_t numBytes = sizeof(list) + (arr_size * t_size);
    list * l = malloc(numBytes);
    l->data = (void*)(l+1);
    l->size = 0;
    l->capacity = arr_size;

    return l;
}

/**
 * Frees a list struct
 * @param l a pointer to the list to free
 */
void free_list(list *l) {
    if(!l) return; //Early exit on null pointer
    free(l->data);
    free(l);
}

/**
 * Initialises an array struct on the heap
 * @param elem_size the size of the elements the array stores in bytes
 * @param size the number of elements in the array
 * @return a pointer to the created array
 */
array* array_init_interal(size_t elem_size, size_t size) {
    array* arr = malloc(sizeof(array));

    arr->size = size;
    arr->data = malloc(size * elem_size);

    //Handle mis-allocation errors
    if(arr->data == NULL) {
        free(arr);
        arr = NULL;
    }

    return arr;
}

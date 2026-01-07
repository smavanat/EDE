#include "../include/list.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void grow(list *slice, size_t size) {
    size_t new_capacity = slice->capacity ? slice->capacity * 2 : 1;
    //Checking if the dynamic array is the last allocation in the arena
    void *data = malloc(new_capacity*size);
    if(slice->size > 0) {
        memcpy(data, slice->data, slice->size*size);
    }
    slice->data = data;
    slice->capacity = new_capacity;
}

list *list_alloc(size_t arr_size, size_t t_size) {
    size_t numBytes = sizeof(list) + (arr_size * t_size);
    list * l = malloc(numBytes);
    l->data = (void*)(l+1);
    l->size = 0;
    l->capacity = arr_size;

    return l;
}

void free_list(list *l) {
    l->size = 0;
    l->capacity = 0;
    free(l->data);
    free(l);
}

array* array_init_interal(size_t elem_size, size_t size) {
    array* arr = malloc(sizeof(array));

    arr->size = size;
    arr->data = malloc(size * elem_size);

    if(arr->data == NULL) {
        free(arr);
        arr = NULL;
    }

    return arr;
}

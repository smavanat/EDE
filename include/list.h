#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    size_t size;
    size_t capacity;
    void* data;
} list;

void grow(list *slice, size_t size);

#define remove_val(list, type, val) do {            \
        for(int i = 0; i < list->size; i++) {       \
            if(((type*)(list)->data)[i] == (val)){  \
                (list)->size--;                     \
                ((type*)(list)->data)[i] = ((type*)(list)->data)[(list)->size];    \
                break;                              \
            }                                       \
        }                                           \
    } while(0);

#define remove_at(list, type, pos) do {             \
        (list)->size--;                             \
        ((type*)(list)->data)[pos] = ((type*)(list)->data)[(list)->size];\
    } while(0);

#define push_value(lst, type, val) do { \
    if ((lst)->size >= (lst)->capacity) \
        grow((lst), sizeof(type)); \
    ((type*)(lst)->data)[(lst)->size++] = (val); \
} while (0)


list *list_alloc(size_t arr_size, size_t t_size);

typedef struct {
    void* data;
    size_t size;
} array;

array* array_init_interal(size_t elem_size, size_t size);

#endif

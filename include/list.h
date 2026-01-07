#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    size_t size;
    size_t capacity;
    void* data;
} list;

//Increases the size of the list once capacity is reached
void grow(list *slice, size_t size);

//Macro for removing a value from the list by value. Does not maintain order
#define remove_val(list, type, val) do {                                            \
        for(int i = 0; i < list->size; i++) {                                       \
            if(((type*)(list)->data)[i] == (val)){                                  \
                (list)->size--;                                                     \
                ((type*)(list)->data)[i] = ((type*)(list)->data)[(list)->size];     \
                break;                                                              \
            }                                                                       \
        }                                                                           \
    } while(0);

//Macro for removing a value from the list by index. Does not maintain order
#define remove_at(list, type, pos) do {                                             \
        if(pos < 0 || pos >= (list->size) || (list)->size == 0) break;              \
        (list)->size--;                                                             \
        ((type*)(list)->data)[pos] = ((type*)(list)->data)[(list)->size];           \
    } while(0);

//Macro for adding a value to the list
#define push_value(lst, type, val) do {                                             \
    if ((lst)->size >= (lst)->capacity)                                             \
        grow((lst), sizeof(type));                                                  \
    ((type*)(lst)->data)[(lst)->size++] = (val);                                    \
} while (0)

//Getting a value from the list
#define get_value(lst, type, index) ((type *)lst->data)[index]

//Allocation function
list *list_alloc(size_t arr_size, size_t t_size);
void free_list(list *l);

//Not really used but could be useful?
typedef struct {
    void* data;
    size_t size;
} array;

array* array_init_interal(size_t elem_size, size_t size);

#endif

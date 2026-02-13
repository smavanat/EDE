#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    size_t size;
    size_t capacity;
    void* data;
} list;

/**
 * Grows the given list by a factor of 2. Only used when the list is at capacity
 * @param slice the list to grow
 * @param size the size of the elements in the list in bytes
 */
void grow(list *slice, size_t size);

/**
 * Macro for removing a value from the list by value. Does not maintain order. Only removes the first occurence of a value
 * @param list the list to remove a value from
 * @param type the type of element being removed. Must match the type already stored in the list
 * @param val the value to remove
 */
#define remove_val(list, type, val) do {                                            \
        for(int i = 0; i < list->size; i++) {                                       \
            if(((type*)(list)->data)[i] == (val)){                                  \
                (list)->size--;                                                     \
                ((type*)(list)->data)[i] = ((type*)(list)->data)[(list)->size];     \
                break;                                                              \
            }                                                                       \
        }                                                                           \
    } while(0);

/**
 * Macro for removing a value from the list by index. Does not maintain order
 * @param list the list to remove a value from
 * @param type the type of element being removed. Must match the type already stored in the list
 * @param pos the index of the value to remove
 */
#define remove_at(list, type, pos) do {                                             \
        if(pos < 0 || pos >= (list->size) || (list)->size == 0) break;              \
        (list)->size--;                                                             \
        ((type*)(list)->data)[pos] = ((type*)(list)->data)[(list)->size];           \
    } while(0);

/**
 * Macro for adding a value to a list
 * @param lst the list to add a value to
 * @param type the type of the value to add. Must match the type already stored in the list
 * @param val the value to add
 */
#define push_value(lst, type, val) do {                                             \
    if ((lst)->size >= (lst)->capacity)                                             \
        grow((lst), sizeof(type));                                                  \
    ((type*)(lst)->data)[(lst)->size++] = (val);                                    \
} while (0)

//
/**
 * Macro for getting a value from the list
 * @param lst the list to get a value from
 * @param type the type of the value to get. Must match the type already stored in the list
 * @param val the index of the value to get
 */
#define get_value(lst, type, index) ((type *)lst->data)[index]

/**
 * Allocates a new list
 * @param arr_size the initial number of elements the list should have
 * @param t_size the size of the type the list should store
 * @return a pointer to the created list on the heap
 */
list *list_alloc(size_t arr_size, size_t t_size);
/**
 * Frees a list struct
 * @param l a pointer to the list to free
 */
void free_list(list *l);

//Not really used but could be useful?
typedef struct {
    void* data;
    size_t size;
} array;

/**
 * Initialises an array struct on the heap
 * @param elem_size the size of the elements the array stores in bytes
 * @param size the number of elements in the array
 * @return a pointer to the created array
 */
array* array_init_interal(size_t elem_size, size_t size);

#endif

#include "../include/queue.h"
#include <stdlib.h>
#include <string.h>

/**
 * Grows the queue when it reaches capacity by a factor of 2
 * @param slice the queue to grow
 * @param size the size of an individual element in the queue in bytes
 */
void grow_queue(queue * slice, size_t size) {
    size_t new_capacity = (slice->capacity > 0) ? slice->capacity * 2 : 1; //If the queue capacity is 0, set it to 1
    char *new_data = malloc(new_capacity * size); //New data buffer

    //Copy over the old data into the new buffer in the correct order
    if (slice->size > 0) {
        for (size_t i = 0; i < slice->size; ++i) {
            size_t idx = (slice->frontptr + i) % slice->capacity;
            memcpy(new_data + i * size, (char*)slice->data + idx * size, size);
        }
    }

    free(slice->data);
    slice->data = new_data;
    slice->capacity = new_capacity;
    slice->frontptr = 0;
    slice->backptr = slice->size;
}

/**
 * Allocates a new queue struct and returns its pointer
 * @param arr_size the number of elements in the new queue
 * @param t_size the size of an individual queue element in bytes
 * @return a pointer to the created queue struct
 */
queue *queue_alloc(size_t arr_size, size_t t_size) {
    queue *q = malloc(sizeof(queue));
    q->data= malloc(arr_size * t_size);
    q->backptr = 0;
    q->frontptr = 0;
    q->size = 0;
    q->capacity = arr_size;
    return q;
}

/**
 * Frees a queue struct
 * @param q a pointer to the queue struct to free
 */
void free_queue(queue *q) {
    free(q->data);
    q->backptr = 0;
    q->frontptr = 0;
    q->capacity = 0;
    q->size = 0;
    free(q);
}

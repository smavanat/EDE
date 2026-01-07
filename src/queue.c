#include "../include/queue.h"
#include <stdlib.h>
#include <string.h>

void grow_queue(queue * slice, size_t size) {
    size_t new_capacity = slice->capacity ? slice->capacity * 2 : 1;
    //Checking if the dynamic array is the last allocation in the arena
    void *data = malloc(new_capacity*size);
    if(slice->size > 0) {
        // Copy elements in correct order
        for (size_t i = 0; i < slice->size; i++) {
            memcpy(&data[i], &slice->data[(slice->frontptr + i) % slice->capacity], size);
        }
    }
    slice->data = data;
    slice->capacity = new_capacity;
}

queue *queue_alloc(size_t arr_size, size_t t_size) {
    queue *q = malloc(sizeof(queue));
    q->data= malloc(arr_size * t_size);
    q->backptr = 0;
    q->frontptr = 0;
    q->size = 0;
    q->capacity = arr_size;
    return q;
}

//Destroys a queue struct
void free_queue(queue *q) {
    free(q->data);
    q->backptr = 0;
    q->frontptr = 0;
    q->capacity = 0;
    q->size = 0;
    free(q);
}

#ifndef __QUEUE_H__
#define __QUEUE_H__
#include <stddef.h>

typedef struct {
    void *data;
    size_t size;
    size_t capacity;
    size_t frontptr; //Index pointer to front of the queue
    size_t backptr; //Index pointer to the end of the queue
} queue;

void grow_queue(queue *slice, size_t size);
queue *queue_alloc(size_t arr_size, size_t t_size);
void free_queue(queue *q);

//Adds a new queue_node to the end of the queue. Expands the buffer for storing queue_nodes if it has run out of space
#define enqueue(q, type, val) do {                                          \
    if ((q)->size >= (q)->capacity)                                         \
        grow_queue((q), sizeof(type));                                      \
    ((type*)(q)->data)[(q)->backptr] = (val);                               \
    q->backptr = (q->backptr + 1) % q->capacity;                            \
    q->size++;                                                              \
} while (0)

//Removes the node from the front of the queue;
#define deqeue(q, type, out) do {                                           \
    if ((q)->size == 0) break;                                              \
    (out) = ((type*)(q)->data)[(q)->frontptr];                              \
    (q)->frontptr = ((q)->frontptr + 1) % (q)->capacity;                    \
    (q)->size--;                                                            \
} while (0)
#endif

#ifndef __QUEUE_H__
#define __QUEUE_H__
#include <stddef.h>

//Implementation of a circular-array based queue
typedef struct {
    void *data;
    size_t size;
    size_t capacity;
    size_t frontptr; //Index pointer to front of the queue
    size_t backptr; //Index pointer to the end of the queue
} queue;

/**
 * Grows the queue when it reaches capacity by a factor of 2
 * @param slice the queue to grow
 * @param size the size of an individual element in the queue in bytes
 */
void grow_queue(queue *slice, size_t size);
/**
 * Allocates a new queue struct and returns its pointer
 * @param arr_size the number of elements in the new queue
 * @param t_size the size of an individual queue element in bytes
 * @return a pointer to the created queue struct
 */
queue *queue_alloc(size_t arr_size, size_t t_size);
/**
 * Frees a queue struct
 * @param q a pointer to the queue struct to free
 */
void free_queue(queue *q);

/**
 * Adds a new queue_node to the end of the queue. Expands the buffer for storing queue_nodes if it has run out of space
 * @param q the queue to enqueue onto
 * @param type the type of element the queue currently stores
 * @param val the value to enqueue
 */
#define enqueue(q, type, val) do {                                          \
    if ((q)->size >= (q)->capacity)                                         \
        grow_queue((q), sizeof(type));                                      \
    ((type*)(q)->data)[(q)->backptr] = (val);                               \
    q->backptr = (q->backptr + 1) % q->capacity;                            \
    q->size++;                                                              \
} while (0)

/**
 * Removes the node from the front of the queue;
 * @param q the queue to dequeue from
 * @param type the type of element the queue currently stores
 * @param out the value returned from dequeuing
 * @param ok whether dequeuing happened successfully or not
 */
#define dequeue(q, type, out, ok) do {                                      \
    if ((q)->size == 0) (ok) = false;                                       \
    else {                                                                  \
        (out) = ((type*)(q)->data)[(q)->frontptr];                          \
        (q)->frontptr = ((q)->frontptr + 1) % (q)->capacity;                \
        (q)->size--;                                                        \
        (ok) = true;                                                        \
    }                                                                       \
} while (0)
#endif

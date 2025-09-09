#ifndef __INT_HASHMAP_H__
#define __INT_HASHMAP_H__

#define EMPTY -1 //Assume all values stored in this hashmap >= 0
#define DUMMY -2

typedef struct {
    int size;
    int capacity;
    int* keys;
    int* values;
} int_hashmap;

int_hashmap *create_hashmap(int size);

int get(int_hashmap* h, int key);
int remove_from_hashmap(int_hashmap* h, int key);
int add(int_hashmap* h, int key, int value);

#endif

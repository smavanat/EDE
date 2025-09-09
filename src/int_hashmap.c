#include "../include/int_hashmap.h"
#include <stdlib.h>

//Checks if n is prime
int is_prime(int n) {
    if(n <= 1) return 0;
    if(n <= 3) return 1;
    if(0 == n % 2 || 0 == n % 3) return 0;

    for(int i = 5; i * i <= n; i +=6) {
        if(n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

//Gets the next prime number after n
int next_prime(int n) {
    if(n <= 2) return 2;
    n = (0 == n % 2) ? n +1 : n; //Make sure n is odd

    while(!is_prime(n)){
        n += 2; //Skip even numbers
    }
    return n;

}

//Creates a hashmap of the given size
//If the provided size is not a prime, the hashmap will be the size of the next prime after the given size
int_hashmap *create_hashmap(int size) {
    int_hashmap *h = malloc(sizeof(int_hashmap));
    int m_size = size;
    if(!is_prime(m_size))
        m_size = next_prime(m_size);
    h->size = 0;
    h->capacity = m_size;
    h->keys = malloc(m_size * sizeof(int));
    h->values = malloc(m_size * sizeof(int));

    return h;
}

//Primary hash funtion.
//key -> the key to be hashed
//length -> the size of the hashmap storing the key
int hash_int(int key, int length) {
    return (key & 0x7fffffff) % length;
}

//Secondary hash funtion.
//key -> the key to be hashed
//length -> the size of the hashmap storing the key
int second_hash_int(int key, int length) {
    return 1 + (key & 0x7fffffff) % (length - 1);
}

//Finds the next slot that we can put a value into in the hashmap
int find_slot(int_hashmap *h, int hash, int key) {
    int avail = -1;
    int j = hash;
    int i = 0;

    do {
        if(h->keys[j] == DUMMY) {
            if(avail == -1) avail = j;
            break;
        }
        else if (h->keys[j] == key){
            return j;
        }
        i++;
        j = (hash + i * second_hash_int(key, h->capacity)) % h->capacity;
    } while(i < h->capacity);

    return -(avail + 1);
}

//Resizes the hashmap to a new size
void resize(int_hashmap *h, int newCap) {
    //Save the old values for rehashing:
    int *oldKeys = h->keys;
    int *oldVals = h->values;
    int oldCap = h->capacity;

    //Update the capcity to the new value:
    h->capacity = newCap;
    h->size = 0; //Reset the size to 0 as it will be naturally incremented in add()

    //Create the new arrays with the new capacity
    h->keys = malloc(newCap * sizeof(int));
    h->values = malloc(newCap * sizeof(int));

    //Rehash and reinsert all entries from the old table into the new one
    for(int i = 0; i < oldCap; i++) {
        if(oldKeys[i] == -1) {
            add(h, oldKeys[i], oldVals[i]);
        }
    }
    free(oldKeys);
    free(oldVals);
}

//Gets a value from a int_hashmap
//h -> the int_hashmap to be searched
//key -> the key whose value needs to be retrieved
//Returns -1 if no value is found
int get(int_hashmap* h, int key) {
    int bucketIndex = hash_int(key, h->capacity);
    int j = find_slot(h, bucketIndex, key);
    if(j < 0) return -1;
    return h->values[j];
}

//Removes a kvp from the int_hashmap and returns its value
//h -> the int_hashmap to be searched
//key -> the key whose value needs to be removed
//Returns -1 if no value is found
int remove_from_hashmap(int_hashmap* h, int key) {
    int bucketIndex = hash_int(key, h->capacity);
    int j = find_slot(h, bucketIndex, key);
    if(j < 0) return -1;
    int val = h->values[j];
    h->keys[j] = EMPTY;
    h->values[j] = EMPTY;
    h->size--;
    return val;
}

//Adds a kvp to the int_hashmap, replacing the value if the key already exists in the hashmap
//h -> the int_hashmap to be searched
//key -> the key which needs to be stored
//value -> the value which needs to be stored
//Returns -1 if the key does not already exist, or the old value if it does.
int add(int_hashmap* h, int key, int value) {
    if(key < 0) return -1; //Do not allow keys that are less than zero as that will mess things up

    int bucketIndex = hash_int(key, h->capacity);
    int j = find_slot(h, bucketIndex, key);

    if(j >= 0) {
        int oldVal = h->values[j];
        h->values[j] = value;
        return oldVal;
    }
    h->keys[-(j+1)] = key;
    h->values[-(j+1)] = value;
    h->size++;

    //If we are above the load factor of 0.75, we need to resize the hashmap
    if(h->size > h->capacity * 0.75)
        resize(2 * h->capacity);
    return -1;
}

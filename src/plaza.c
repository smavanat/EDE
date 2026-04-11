#include "../include/plaza.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Initialises a plaza and all of its constituent components, namely the component_array storing data for each type of component,
 * the array of entity signatures, the archetype array of all entity archetypes, and the entity pool allocator
 * @return a pointer to the created plaza
 */
plaza *init_plaza(void) {
    plaza *p = malloc(sizeof(plaza)); //Allocating the plaza on the heap

    //Entity Allocator
    p->entities = malloc(sizeof(entity_pool)); //Creating the entity pool allocator
    //Allocating the entities in the pool
    for(int i = 1; i < MAX_ENTITIES; i++) {
        p->entities->freeList[i] = MAX_ENTITIES - i - 1; //Reverse order for reuse;
    }
    p->entities->top = MAX_ENTITIES - 1; //Setting the first entity so we can use the reverse order
    p->entities->count = 0; //Count of used entities is 0 at the start

    //Component arrays
    p->componentArrays = malloc(sizeof(component_array *) * NUM_COMPONENTS); //Set the number of component arrays to be the number of components currently used
    //Initialise each component array
    for(int i = 0; i < NUM_COMPONENTS; i++) {
        int componentSize = 0;
        void (*free_func) = NULL;
        //Initialise the size and the destructor for each component
        switch (i) {
            case TRANSFORM:
                componentSize = sizeof(transform);
                free_func = &free_transform;
                break;
            case SPRITE:
                componentSize = sizeof(sprite);
                free_func = &free_sprite;
                break;
            case COLLIDER:
                componentSize = sizeof(collider);
                free_func = &free_collider;
                break;
            // case PATHFINDING:
            //     componentSize = sizeof(pathfinding);
            //     free_func = &free_pathfinding;
            //     break;
            case RIGIDBODY:
                componentSize = sizeof(rigidbody);
                free_func = &free_rigidbody;
                break;
            case BUTTON:
                componentSize = sizeof(button);
                free_func = &free_button;
        }
        p->componentArrays[i] = initialise_component_array(componentSize, free_func);
    }

    //Initialise the entity signature array
    p->entitySignatures = malloc(sizeof(signature)*MAX_ENTITIES);
    memset(p->entitySignatures, 0, sizeof(signature) * MAX_ENTITIES);
    //Need to initialise archetypes:
    p->entityArchetypes = list_alloc(16, sizeof(archetype *));
    return p;
}

/**
 * Creates an entity through the plaza
 * @oaram p the plaza to use for creating the entity
 * @return the entity created
 */
entity create_entity(plaza *p) {
    if (p->entities->top < 0) {
        fprintf(stderr, "Entity pool exhausted!\n"); //Used all of the entities
        exit(1);
    }

    entity id = p->entities->freeList[p->entities->top--];
    p->entities->count++; //Increment count of used entities
    return id;
}

/**
 * Destroys an entity
 * @param p the plaza to destroy the entity in
 * @param e the entity to destroy
 */
void destroy_entity(plaza *p, entity e) {
    //If for some reason the entity pool overflows
     if (p->entities->top >= MAX_ENTITIES - 1) {
        fprintf(stderr, "Entity pool overflow!\n");
        exit(1);
    }

    p->entities->freeList[++p->entities->top] = e; //Update the pool of free entities
    p->entities->count--; //Reduce the count of used entities

    //Remove all the components attached to this entity
    for(int i = 0; i < 32; i++) {
        if(p->entitySignatures[e] & (1 << i)) {
            remove_component_from_entity(p, e, i);
        }
    }
    //Reset the entity signature
    p->entitySignatures[e] = 0;
}

/**
 * Adds a component to an entity
 * @param p the plaza where the component needs to be added
 * @param e the entity to add a component to
 * @param t the component type we are adding to
 * @param c a pointer to the component data to add
 */
void add_component_to_entity(plaza *p, entity e, component_type t, void *c) {
    //Removing entity from its old archetype:
    if(p->entitySignatures[e] != 0) {
        for(size_t i = 0; i < p->entityArchetypes->size; i++) {
            if(get_value(p->entityArchetypes, archetype *, i)->signature == p->entitySignatures[e]) { //Iterate over all of the archetypes stored in the plaza
                //This is very inefficient. When doing chunking store a sparse set that uses 
                //an int32 split into three parts:  one to represent the archetype that the entity is in
                //                                  one to represent the chunk the entity is in
                //                                  one to represent the index of the entity in the chunk
                //Each part can be 8 bits
                //This will lead to much faster lookup
                for(size_t j = 0; j < p->entityArchetypes->size; j++) { //Iterate over all of the entities in an archetype
                    if(e == get_value(p->entityArchetypes, archetype *, i)->entities[j]) { //If this is the entity that we want
                        get_value(p->entityArchetypes, archetype *, i)->entities[j] = get_value(p->entityArchetypes, archetype *, i)->entities[p->entityArchetypes->size-1]; //Remove the entity by swapping it with the last element of the entity array in the archetype
                        break;
                    }
                }
                get_value(p->entityArchetypes, archetype *, i)->size--; //Reduce the size of the array of entities in the archetype by 1
                break;
            }
        }
    }

    add_component_runtime(p->componentArrays[t], e, c); //Add component to the entity in the component array
    p->entitySignatures[e] |= (1 << t); //Add a bit to the signature

    //Adding an entity to a specific archetype
    bool foundSig = false;
    for(size_t i = 0; i < p->entityArchetypes->size; i++) { //Iterate over all of the entity archetypes
        if(get_value(p->entityArchetypes, archetype *, i)->signature == p->entitySignatures[e]) { //If the entity's signature matches the archetypes signature
            foundSig = true;
            //Add the entity
            archetype *arch = get_value(p->entityArchetypes, archetype *, i);
            arch->entities[arch->size] = e;
            arch->size++;
            break;
        }
    }

    //Need to append new archetype to end of entity archetypes because none exist that match this entity's archetypes
    if(!foundSig) {
        archetype *temp = malloc(sizeof(archetype));
        temp->signature = p->entitySignatures[e];
        temp->size = 1;
        temp->entities[0] = e;
        push_value(p->entityArchetypes, archetype *, temp);
    }
}

/**
 * Removes a component from an entity
 * @param p the plaza that the entity is stored in
 * @param e the entity to remove the component from
 * @parma t the type of component to remove
 */
void remove_component_from_entity(plaza *p, entity e, component_type t) {
    //Removing entity from its old archetype:
    for(size_t i = 0; i < p->entityArchetypes->size; i++) { //Iterate over all of the archetypes
        if(get_value(p->entityArchetypes, archetype *, i)->signature == p->entitySignatures[e]) { //If the achetype signature matches the entity signature
            //This is very inefficient. When doing chunking store a sparse set that uses
            //an int32 split into three parts:  one to represent the archetype that the entity is in
            //                                  one to represent the chunk the entity is in
            //                                  one to represent the index of the entity in the chunk
            //Each part can be 8 bits
            //This will lead to much faster lookup
            archetype *arch = get_value(p->entityArchetypes, archetype *, i); //Get the archetype
            for(size_t j = 0; j < arch->size; j++) { //Iterate over all the entities in the archetype
                //Get rid of the entity by swapping with the last element
                if(e == arch->entities[j]) {
                    arch->entities[j] = arch->entities[arch->size-1];
                    arch->size--;
                    break;
                }
            }
            break;
        }
    }

    remove_component(p->componentArrays[t], e); //Remove the component from the component array
    p->entitySignatures[e] &= ~(1 << t); //Remove a bit from the signature

    if(p->entitySignatures[e] != 0) { //If the entity still has components attached to it
        //Adding an entity to a specific archetype
        bool foundSig = false;
        //Finding if there is an existing archetype for this entity
        for(size_t i = 0; i < p->entityArchetypes->size; i++) { //Iterate over all of the entity archetypes
            if(get_value(p->entityArchetypes, archetype *, i)->signature == p->entitySignatures[e]) { //If the archetype signature matches the entity signature
                foundSig = true;
                //Add it to the archetype's entity array
                archetype *arch = get_value(p->entityArchetypes, archetype *, i);
                arch->entities[arch->size] = e;
                arch->size++;
                break;
            }
        }

        //Need to append new archetype to end of entity archetypes if none exist for this entity's signature
        if(!foundSig) {
            archetype *temp = malloc(sizeof(archetype));
            temp->signature = p->entitySignatures[e];
            temp->size = 1;
            temp->entities[0] = e;
            push_value(p->entityArchetypes, archetype*, temp);
        }
    }
}

/**
 * Returns a pointer to a component of a specific type from an entity
 * @param p the plaza where this occurs
 * @param e the entity to get the component from
 * @param t the type of the component to retrieve
 */
void *get_component_from_entity(plaza *p, entity e, component_type t) {
    return get_component(p->componentArrays[t], e);
}

/**
 * Returns an array of all the archetypes that match the given signature
 * @param p the plaza where this operation should occur
 * @param s the signature to query
 */
archetype_array *query_signature(plaza *p, signature s) {
    // int count = 0;
    // for(size_t i = 0; i < p->entityArchetypes->size; i++) {
    //     if((get_value(p->entityArchetypes, archetype *, i)->signature & s) == s) {
    //         count++;
    //     }
    // }
    archetype_array *ret = list_alloc(10, sizeof(archetype *)); //Array of archetype pointers that we return

    for(size_t i = 0; i < p->entityArchetypes->size; i++) { //Iterate over all of the entity archetypes
        if((get_value(p->entityArchetypes, archetype *, i)->signature & s) == s) { //If the signature matches the desired one, add it to the returned array
            push_value(ret, archetype *, get_value(p->entityArchetypes, archetype *, i));
        }
    }

    return ret;
}

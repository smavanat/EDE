#include "../include/plaza.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

plaza *init_plaza(void) {
    plaza *p = malloc(sizeof(plaza));
    p->entities = malloc(sizeof(entity_pool));
    for(int i = 0; i < MAX_ENTITIES; i++) {
        p->entities->freeList[i] = MAX_ENTITIES - i - 1; //Reverse order for reuse;
    }
    p->entities->top = MAX_ENTITIES - 1;
    p->entities->count = 0;
    p->componentArrays = malloc(sizeof(component_array *) * NUM_COMPONENTS);
    for(int i = 0; i < NUM_COMPONENTS; i++) {
        int componentSize = 0;
        switch (i) {
            case TRANSFORM:
                componentSize = sizeof(transform);
                break;
            case SPRITE:
                componentSize = sizeof(sprite);
                break;
            case COLLIDER:
                componentSize = sizeof(collider);
                break;
            case PATHFINDING:
                componentSize = sizeof(pathfinding);
                break;
            case PIXEL:
                componentSize = sizeof(pixel);
                break;
            case RIGIDBODY:
                componentSize = sizeof(rigidbody);
                break;
        }
        p->componentArrays[i] = initialise_component_array(componentSize);
    }
    p->entitySignatures = malloc(sizeof(signature)*MAX_ENTITIES);
    memset(p->entitySignatures, 0, sizeof(signature) * MAX_ENTITIES);
    //Need to initialise archetypes:
    p->entityArchetypes = list_alloc(16, sizeof(archetype *));
    return p;
}
entity create_entity(plaza *p) {
    // entity e = p->header->next->val;
    // entity_pool *old = p->header->next;
    // p->header->next = old->next;
    // free(old);
    // return e;
    if (p->entities->top < 0) {
        fprintf(stderr, "Entity pool exhausted!\n");
        exit(1);
    }

    entity id = p->entities->freeList[p->entities->top--];
    p->entities->count++;
    return id;
}

void destroy_entity(plaza *p, entity e) {
     if (p->entities->top >= MAX_ENTITIES - 1) {
        fprintf(stderr, "Entity pool overflow!\n");
        exit(1);
    }

    p->entities->freeList[++p->entities->top] = e;
    p->entities->count--;   // p->tailer = new;

    //Remove all the components attached to this entity
    for(int i = 0; i < 32; i++) {
        if(p->entitySignatures[e] & (1 << i)) {
            remove_component_from_entity(p, e, i);
        }
    }
    //Reset the entity signature
    p->entitySignatures[e] = 0;
}

void add_component_to_entity(plaza *p, entity e, component_type t, void *c) {
    //Removing entity from its old archetype:
    if(p->entitySignatures[e] != 0) {
        for(size_t i = 0; i < p->entityArchetypes->size; i++) {
            if(get_value(p->entityArchetypes, archetype *, i)->signature == p->entitySignatures[e]) {
                //This is very inefficient. When doing chunking store a sparse set that uses 
                //an int32 split into three parts:  one to represent the archetype that the entity is in
                //                                  one to represent the chunk the entity is in
                //                                  one to represent the index of the entity in the chunk
                //Each part can be 8 bits
                //This will lead to much faster lookup
                for(size_t j = 0; j < p->entityArchetypes->size; j++) {
                    if(e == get_value(p->entityArchetypes, archetype *, i)->entities[j]) {
                        get_value(p->entityArchetypes, archetype *, i)->entities[j] = get_value(p->entityArchetypes, archetype *, i)->entities[p->entityArchetypes->size-1];
                        break;
                    }
                }
                get_value(p->entityArchetypes, archetype *, i)->size--;
                break;
            }
        }
    }

    add_component_runtime(p->componentArrays[t], e, c);
    p->entitySignatures[e] |= (1 << t); //Add a bit to the signature

    //Adding an entity to a specific archetype
    bool foundSig = false;
    for(size_t i = 0; i < p->entityArchetypes->size; i++) {
        if(get_value(p->entityArchetypes, archetype *, i)->signature == p->entitySignatures[e]) {
            foundSig = true;
            archetype *arch = get_value(p->entityArchetypes, archetype *, i);
            arch->entities[arch->size] = e;
            arch->size++;
            break;
        }
    }

    //Need to append new archetype to end of entity archetypes
    if(!foundSig) {
        archetype *temp = malloc(sizeof(archetype));
        temp->signature = p->entitySignatures[e];
        temp->size = 1;
        temp->entities[0] = e;
        push_value(p->entityArchetypes, archetype *, temp);
    }
}

void remove_component_from_entity(plaza *p, entity e, component_type t) {
    //Removing entity from its old archetype:
    for(size_t i = 0; i < p->entityArchetypes->size; i++) {
        if(get_value(p->entityArchetypes, archetype *, i)->signature == p->entitySignatures[e]) {
            //This is very inefficient. When doing chunking store a sparse set that uses 
            //an int32 split into three parts:  one to represent the archetype that the entity is in
            //                                  one to represent the chunk the entity is in
            //                                  one to represent the index of the entity in the chunk
            //Each part can be 8 bits
            //This will lead to much faster lookup
            for(size_t j = 0; j < p->entityArchetypes->size; j++) {
                if(e == get_value(p->entityArchetypes, archetype *, i)->entities[j]) {
                    get_value(p->entityArchetypes, archetype *, i)->entities[j] = get_value(p->entityArchetypes, archetype *, i)->entities[p->entityArchetypes->size-1];
                    break;
                }
            }
            get_value(p->entityArchetypes, archetype *, i)->size--;
            break;
        }
    }

    remove_component(p->componentArrays[t], e);
    p->entitySignatures[e] &= ~(1 << t); //Remove a bit from the signature

    if(p->entitySignatures[e] != 0) {
        //Adding an entity to a specific archetype
        bool foundSig = false;
        for(size_t i = 0; i < p->entityArchetypes->size; i++) {
            if(get_value(p->entityArchetypes, archetype *, i)->signature == p->entitySignatures[e]) {
                foundSig = true;
                archetype *arch = get_value(p->entityArchetypes, archetype *, i);
                arch->entities[arch->size] = e;
                arch->size++;
                break;
            }
        }

        //Need to append new archetype to end of entity archetypes
        if(!foundSig) {
            archetype *temp = malloc(sizeof(archetype));
            temp->signature = p->entitySignatures[e];
            temp->size = 1;
            temp->entities[0] = e;
            push_value(p->entityArchetypes, archetype*, temp);
        }
    }
}

void *get_component_from_entity(plaza *p, entity e, component_type t) {
    return get_component(p->componentArrays[t], e);
}

//Returns an array of all the archetypes that match the given signature
archetype_array *query_signature(plaza *p, signature s) {
    int count = 0;
    for(size_t i = 0; i < p->entityArchetypes->size; i++) {
        if((get_value(p->entityArchetypes, archetype *, i)->signature & s) == s) {
            count++;
        }
    }
    archetype_array *ret = list_alloc(count, sizeof(archetype *));

    for(size_t i = 0; i < p->entityArchetypes->size; i++) {
        if((get_value(p->entityArchetypes, archetype *, i)->signature & s) == s) {
            push_value(ret, archetype *, get_value(p->entityArchetypes, archetype *, i));
        }
    }

    return ret;
}

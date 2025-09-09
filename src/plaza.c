#include "../include/plaza.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

plaza *init_plaza(void) {
    plaza *p = malloc(sizeof(plaza));
    // p->header = malloc(sizeof(entity_pool));
    // entity_pool *current = p->header;
    // for(int i = 0; i < MAX_ENTITIES; i++) {
    //     current->next = malloc(sizeof(entity_pool));
    //     current->next->val = i;
    //     current = current->next;
    // }
    // p->tailer = malloc(sizeof(entity_pool));
    // current->next = NULL;
    // p->tailer = current;

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
            case TILE_SPRITE:
                componentSize = sizeof(tile_sprite);
                break;
            case COLLIDER:
                componentSize = sizeof(collider);
                break;
            case TAG:
                componentSize = sizeof(tag);
                break;
            case TILE_RECT:
                componentSize = sizeof(tile_rect);
                break;
            case PATHFINDING:
                componentSize = sizeof(pathfinding);
                break;
            case TERRAIN:
                componentSize = sizeof(terrain);
                break;
        }
        p->componentArrays[i] = initialise_component_array(componentSize);
    }
    p->entitySignatures = malloc(sizeof(signature)*MAX_ENTITIES);
    memset(p->entitySignatures, 0, sizeof(signature) * MAX_ENTITIES);

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
    // entity_pool *new = malloc(sizeof(entity_pool));
    // new->val = e;
    // new->next = NULL;
    // p->tailer->next = new;
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
    add_component_runtime(p->componentArrays[t], e, c);
    p->entitySignatures[e] = p->entitySignatures[e] | (1 << t); //Add a bit to the signature
}

void remove_component_from_entity(plaza *p, entity e, component_type t) {
    remove_component(p->componentArrays[t], e);
    p->entitySignatures[e] = ~(1 << t); //Remove a bit from the signature
}

void *get_component_from_entity(plaza *p, entity e, component_type t) {
    return get_component(p->componentArrays[t], e);
}

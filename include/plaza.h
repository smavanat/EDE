#ifndef __PLAZA_H__
#define __PLAZA_H__
#include "component.h"
#include "component_array.h"
#include "entity.h"
#include "list.h"
#include <stdint.h>
//This is where all of our entities and components are managed. Hence plaza, because its where they all congregate
typedef uint32_t signature;

//Linked list node to represent a pool allocator for entities
typedef struct{
    int top;
    int count;
    entity freeList[MAX_ENTITIES];
} entity_pool;

//For grouping entities by their component types;
typedef struct {
    signature signature;
    int size;
    entity entities[MAX_ENTITIES];
} archetype;

typedef list archetype_array;

typedef struct {
    entity_pool *entities;
    signature *entitySignatures;
    component_array **componentArrays;
    archetype_array *entityArchetypes;
} plaza;

plaza *init_plaza(void);
entity create_entity(plaza *p);
void destroy_entity(plaza *p, entity e);
void add_component_to_entity(plaza *p, entity e, component_type t, void *c);
void remove_component_from_entity(plaza *p, entity e, component_type t);
void *get_component_from_entity(plaza *p, entity e, component_type t);
archetype_array *query_signature(plaza *p, signature s);

#endif

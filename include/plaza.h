#ifndef __PLAZA_H__
#define __PLAZA_H__
#include "component.h"
#include "component_array.h"
#include "entity.h"
#include "list.h"
#include <stdint.h>
//This is where all of our entities and components are managed. Hence plaza, because its where they all congregate
typedef uint32_t signature;

//A pool allocator for entities
typedef struct{
    int top; //The next entity element to be allocated
    int count; //The number of entities currently used
    entity freeList[MAX_ENTITIES]; //Pool of available entities
} entity_pool;

//For grouping entities by their component types;
typedef struct {
    signature signature; //The signature of entities in the archetype
    size_t size; //Size of the archetype
    entity entities[MAX_ENTITIES]; //Buffer to store the entities
} archetype;

typedef list archetype_array; //An array of archetypes

//This struct manages all of the entites and components in the ecs
typedef struct {
    entity_pool *entities; //The entity allocator
    signature *entitySignatures; //The signatures of all the entities in existance
    component_array **componentArrays; //An array of component arrays
    archetype_array *entityArchetypes; //An array of archetypes
} plaza;

/**
 * Initialises a plaza and all of its constituent components, namely the component_array storing data for each type of component,
 * the array of entity signatures, the archetype array of all entity archetypes, and the entity pool allocator
 * @return a pointer to the created plaza
 */
plaza *init_plaza(void);
/**
 * Creates an entity through the plaza
 * @oaram p the plaza to use for creating the entity
 * @return the entity created
 */
entity create_entity(plaza *p);
/**
 * Destroys an entity
 * @param p the plaza to destroy the entity in
 * @param e the entity to destroy
 */
void destroy_entity(plaza *p, entity e);
/**
 * Adds a component to an entity
 * @param p the plaza where the component needs to be added
 * @param e the entity to add a component to
 * @param t the component type we are adding to
 * @param c a pointer to the component data to add
 */
void add_component_to_entity(plaza *p, entity e, component_type t, void *c);
/**
 * Removes a component from an entity
 * @param p the plaza that the entity is stored in
 * @param e the entity to remove the component from
 * @parma t the type of component to remove
 */
void remove_component_from_entity(plaza *p, entity e, component_type t);
/**
 * Returns a pointer to a component of a specific type from an entity
 * @param p the plaza where this occurs
 * @param e the entity to get the component from
 * @param t the type of the component to retrieve
 */
void *get_component_from_entity(plaza *p, entity e, component_type t);
/**
 * Returns an array of all the archetypes that match the given signature
 * @param p the plaza where this operation should occur
 * @param s the signature to query
 */
archetype_array *query_signature(plaza *p, signature s);

#endif

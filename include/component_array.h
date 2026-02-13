#ifndef __COMPONENT_ARRAY_H__
#define __COMPONENT_ARRAY_H__
#include "entity.h"
#include <stdlib.h>

//Sparse set component_array type
typedef struct{
    int size;
    int componentSize; //The size of the component stored in this array in bytes
    int *sparse;    //Entity id ->index in dense
    int *dense;     //Index -> entity id
    void *componentArray; //Index ->actual component
    void (*free_func)(void *component); // Function to actually free the components once we're done with them
} component_array;

/**
 * Initialises a component array on the heap and returns a pointer to it
 * @param componentSize the size in bytes of an individual component stored in the array
 * @param free_func a function pointer to the function to destroy the component when its no longer needed
 * @return a pointer to the created component_array
 */
component_array *initialise_component_array(int componentSize, void (*free_func)(void *component));
/**
 * Adds a component to a given entity in this component array. Does not use macros, and uses memcpy instead
 * @param cArr the component array where the component needs to be added
 * @param e the id of the entity the component needs to be added to
 * @param c a pointer to the component that needs to be added
 * @return 1 on success, 0 on failure
 */
int add_component_runtime(component_array *cArr, entity e, void *c);
/**
 * Removes a component from the component array by replacing it with the last element of the component array (if there is one)
 * @param cArr the component array where the component needs to be removed
 * @param e the id of the entity whose entity needs to be removed
 * @return 1 on success, 0 on failure
 */
int remove_component(component_array *cArr, entity e);
/**
 * Returns a pointer to the component in the component array if one exists. Uses assert to determine that this is true
 * @param cArr the component array where retrieval needs to occur
 * @param e the entity whose component needs to retrieved
 * @return a pointer to the desired component
 */
void *get_component(component_array *cArr, entity e);
/**
 * Removes a component from the component array, checking that it exists in the sparse set first
 * @param cArr the component array to remove from
 * @param e the entity whose component should be removed
 */
void entity_destroyed(component_array *cArr, entity e);

#endif

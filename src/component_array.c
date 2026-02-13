#include "../include/component_array.h"
#include <string.h>
#include <assert.h>

/**
 * Initialises a component array on the heap and returns a pointer to it
 * @param componentSize the size in bytes of an individual component stored in the array
 * @param free_func a function pointer to the function to destroy the component when its no longer needed
 * @return a pointer to the created component_array
 */
component_array *initialise_component_array(int componentSize, void (*free_func)(void *component)) {
    component_array *c = malloc(sizeof(component_array));
    c->sparse = malloc(sizeof(int)*MAX_ENTITIES);
    memset(c->sparse, -1, sizeof(int)*MAX_ENTITIES);
    c->dense = malloc(sizeof(int)*MAX_ENTITIES);
    c->componentArray = malloc(componentSize*MAX_ENTITIES);
    c->size = 0;
    c->componentSize = componentSize;
    c->free_func = free_func;
    return c;
}

/**
 * Adds a component to a given entity in this component array. Does not use macros, and uses memcpy instead
 * @param cArr the component array where the component needs to be added
 * @param e the id of the entity the component needs to be added to
 * @param c a pointer to the component that needs to be added
 * @return 1 on success, 0 on failure
 */
int add_component_runtime(component_array *cArr, entity e, void *c) {
    if (cArr->sparse[e] != -1) return 0; //Check the sparse array to make sure the entity does not already have a component of this type

    //Add the component to the end of the 'dense' part of the sparse array and update the 'sparse' seciton to reflect this
    int index = cArr->size;
    cArr->sparse[e] = index;
    cArr->dense[index] = e;

    //Copy the actual component over into the array
    void *target = (char *)cArr->componentArray + index * cArr->componentSize;
    memcpy(target, c, cArr->componentSize);

    cArr->size++; //Increment size
    return 1;
}

/**
 * Removes a component from the component array by replacing it with the last element of the component array (if there is one)
 * @param cArr the component array where the component needs to be removed
 * @param e the id of the entity whose entity needs to be removed
 * @return 1 on success, 0 on failure
 */
int remove_component(component_array *cArr, entity e) {
        //If the array does not contain any components related to this entity, return false;
        if(cArr->sparse[e] == -1) return 0;

        //Replace component to be removed with last element in the array and set the last element to null
        //if not removing the last element
        int removalIndex = cArr->sparse[e];
        cArr->free_func((char *)cArr->componentArray + (removalIndex * cArr->componentSize));
        if(removalIndex != cArr->size - 1) {
            void *target = (char *)cArr->componentArray + removalIndex * cArr->componentSize;
            void *source = (char *)cArr->componentArray + (cArr->size - 1) * cArr->componentSize;
            memcpy(target, source, cArr->componentSize);

            //Need to update the mappings to reflect the moved entity
            entity replacingEntity = cArr->dense[cArr->size-1];
            cArr->sparse[replacingEntity] = removalIndex;
            cArr->dense[removalIndex] = replacingEntity;
        }
        //Erase the remvoed entity
        cArr->sparse[e] = -1;
        cArr->size--;
        return 1;
}

/**
 * Returns a pointer to the component in the component array if one exists. Uses assert to determine that this is true
 * @param cArr the component array where retrieval needs to occur
 * @param e the entity whose component needs to retrieved
 * @return a pointer to the desired component
 */
void *get_component(component_array *cArr, entity e) {
    assert(cArr->sparse[e] != -1 && "Retrieving non-existent component");
    return (char *)cArr->componentArray + (cArr->sparse[e] * cArr->componentSize);
}

/**
 * Removes a component from the component array, checking that it exists in the sparse set first
 * @param cArr the component array to remove from
 * @param e the entity whose component should be removed
 */
void entity_destroyed(component_array *cArr, entity e) {
    if(cArr->sparse[e] != -1) {
        remove_component(cArr, e);
    }
}

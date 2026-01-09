#include "../include/component_array.h"
#include <string.h>
#include <assert.h>

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

int add_component_runtime(component_array *cArr, entity e, void *c) {
    if (cArr->sparse[e] != -1) return 0;

    int index = cArr->size;
    cArr->sparse[e] = index;
    cArr->dense[index] = e;

    void *target = (char *)cArr->componentArray + index * cArr->componentSize;
    memcpy(target, c, cArr->componentSize);

    cArr->size++;
    return 1;
}

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
        cArr->sparse[e] = -1;       // cArr->sparse[cArr->size-1] = -1;
        cArr->size--;
        return 1;
}

void *get_component(component_array *cArr, entity e) {
    assert(cArr->sparse[e] != -1 && "Retrieving non-existent component");
    // return &cArr->componentArray[cArr->sparse[e]];
    return (char *)cArr->componentArray + (cArr->sparse[e] * cArr->componentSize);
}

void entity_destroyed(component_array *cArr, entity e) {
    if(cArr->sparse[e] != -1) {
        remove_component(cArr, e);
    }
}

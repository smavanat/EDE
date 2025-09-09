#ifndef __COMPONENT_ARRAY_H__
#define __COMPONENT_ARRAY_H__
#include "entity.h"
#include "int_hashmap.h"
#include <stdlib.h>

//Sparse set component_array type
typedef struct{
    int size;
    int componentSize;
    int *sparse;    //Entity id ->index in dense
    int *dense;     //Index -> entity id
    void *componentArray; //Index ->actual component
} component_array;

#define add_component_to_array(cArr, compType, compVal, pos) do {   \
        ((compType*)(cArr->component_array))[pos] = compVal         \
    } while(0);

#define add_component(cArr, e, compType, compVal) do {              \
        if(cArr->sparse[e] != -1) return 0;                         \
        cArr->sparse[e] = cArr->size;                               \
        cArr->dense[cArr->size] = e;                                \
        ((compType*)(cArr->componentArray))[cArr->size] = compVal;  \
        cArr->size++;                                               \
        return 1;                                                   \
    } while(0);

component_array *initialise_component_array(int componentSize);
int add_component_runtime(component_array *cArr, entity e, void *c);
int remove_component(component_array *cArr, entity e);
void *get_component(component_array *cArr, entity e);
void entity_destroyed(component_array *cArr, entity e);

#endif

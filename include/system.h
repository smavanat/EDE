#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "plaza.h"
typedef struct ecs_system ecs_system;

//Defines template for a system
struct ecs_system{
    void (*init_func)(plaza *p, ecs_system *s); //Function pointer to the initialisation function for this system
    void (*update_func)(plaza *p, ecs_system *s, float dt); //Function pointer to the update function for this system
    archetype_array *archetypes; //Archetypes that match this system's signature
    signature signature; //This system's signature
};

#endif

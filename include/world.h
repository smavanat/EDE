#ifndef __WORLD_H__
#define __WORLD_H__
#include "plaza.h"
#include "system.h"

typedef struct {
    plaza *p;
    list *systems; //NEED TO MAKE THIS A PRIORITY QUEUE SO WE CAN ORDER THE WAY IN WHICH SYSTEMS RUN
    b2WorldId world_id;
} world;

//Creating the world
world *world_alloc(b2WorldId world_id);
//Initialising all the systems stored in the world
void world_init(world *w);
//Updating all the systems stored in the world
void world_update(world *w, float dt);
//Updates all of the systems to have the archetypes that match their signature
void sys_query(world *w);
//Adding a new system to the world with initialisation function init_func and update function update_func
void add_system(world *w, void (*init_func)(plaza *, ecs_system *), void (*update_func)(plaza *, ecs_system *, float));

#endif

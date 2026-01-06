#include "../include/world.h"
#include <stdlib.h>

//Creating the world
world *world_alloc(void) {
    world *ret = malloc(sizeof(world));
    ret->p = init_plaza();
    ret->systems = list_alloc(16, sizeof(ecs_system *));

    return ret;
}

//Initialising all the systems stored in the world
void world_init(world *w) {
    for(int i = 0; i < w->systems->size; i++) {
        ((ecs_system **)w->systems->data)[i]->init_func(w->p, ((ecs_system **)w->systems->data)[i]);
    }
}

//Updating all the systems stored in the world
void world_update(world *w, float dt) {
    for(int i = 0; i < w->systems->size; i ++) {
        ((ecs_system **)w->systems->data)[i]->update_func(w->p, ((ecs_system **)w->systems->data)[i], dt);
    }
}

//Updates all of the systems to have the entities that match their signature
void sys_query(world *w) {
    for(int i = 0; i < w->systems->size; i++) {
        ((ecs_system **)w->systems->data)[i]->archetypes = query_signature(w->p, ((ecs_system **)w->systems->data)[i]->signature);
    }
}

//Adding a new system to the world with initialisation function init_func and update function update_func
void add_system(world *w, void (*init_func)(plaza *, ecs_system *), void (*update_func)(plaza *, ecs_system *, float)) {
    ecs_system *new_sys = malloc(sizeof(ecs_system));
    new_sys->init_func = init_func;
    new_sys->update_func = update_func;
    push_value(w->systems, ecs_system *, new_sys);
}

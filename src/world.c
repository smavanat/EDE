#include "../include/world.h"
#include <stdlib.h>

/*
 * Creating the world
 * @param world_id the id of the box2d world that we are using
 * @return a pointer to the created world struct on the heap
 */
world *world_alloc(b2WorldId world_id) {
    world *ret = malloc(sizeof(world));
    ret->p = init_plaza();
    ret->systems = list_alloc(16, sizeof(ecs_system *));
    ret->world_id = world_id;

    return ret;
}

/*
 * Initialises all the systems stored in the world
 * @param w the world whose systems need to be initialised
 */
void world_init(world *w) {
    for(int i = 0; i < w->systems->size; i++) {
        ((ecs_system **)w->systems->data)[i]->init_func(w->p, ((ecs_system **)w->systems->data)[i]);
    }
}

/*
 * Updating all the systems stored in the world
 * @param w the world whose systems need to be updated
 * @param dt the timestep between the current and previous update
 */
void world_update(world *w, float dt) {
    for(int i = 0; i < w->systems->size; i ++) {
        ((ecs_system **)w->systems->data)[i]->update_func(w->p, ((ecs_system **)w->systems->data)[i], dt);
    }
}

/*
 * Updates all of the systems to have the entities that match their signature
 * @param w the world whose systems need to be updated
 */
void sys_query(world *w) {
    for(int i = 0; i < w->systems->size; i++) {
        ((ecs_system **)w->systems->data)[i]->archetypes = query_signature(w->p, ((ecs_system **)w->systems->data)[i]->signature);
    }
}

/*
 * Adding a new system to the world 
 * @param w the world the system will be added to
 * @param init func the initialisation function for the system
 * @param update_func the update function for the system
 */
void add_system(world *w, void (*init_func)(plaza *, ecs_system *), void (*update_func)(plaza *, ecs_system *, float)) {
    ecs_system *new_sys = malloc(sizeof(ecs_system));
    new_sys->init_func = init_func;
    new_sys->update_func = update_func;
    push_value(w->systems, ecs_system *, new_sys);
}

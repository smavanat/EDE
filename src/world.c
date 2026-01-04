#include "../include/world.h"
#include <stdlib.h>
#include "../include/system.h"

world *world_alloc(void) {
    world *ret = malloc(sizeof(world));
    ret->p = init_plaza();
    ret->systems = list_alloc(16, sizeof(ecs_system *));

    return ret;
}

void world_init(world *w) {
    for(int i = 0; i < w->systems->size; i++) {
        ((ecs_system **)w->systems->data)[i]->init_func(w->p, ((ecs_system **)w->systems->data)[i]);
    }
}

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

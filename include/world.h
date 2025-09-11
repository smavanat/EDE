#ifndef __WORLD_H__
#define __WORLD_H__
#include "plaza.h"

typedef struct {
    plaza *p;
    list *systems;
} world;

world *world_alloc(void);
void world_init(world *w);
void world_update(world *w, float dt);
void sys_query(world *w);

#endif

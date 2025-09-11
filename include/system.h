#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "plaza.h"
typedef struct ecs_system ecs_system;

struct ecs_system{
    void (*init_func)(plaza *p, ecs_system *s);
    void (*update_func)(plaza *p, ecs_system *s, float dt);
    archetype_array *entities;
    signature signature;
};

#endif

#ifndef __BASIC_SYSTEMS_H__
#define __BASIC_SYSTEMS_H__
#include "renderer.h"
#include "system.h"

extern renderer *gRenderer;

void render_system_init(plaza *p, ecs_system *s);
void render_system_update(plaza *p, ecs_system *s, float dt);
#endif

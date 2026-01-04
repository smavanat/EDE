#ifndef __BASIC_SYSTEMS_H__
#define __BASIC_SYSTEMS_H__
#include "component.h"
#include "renderer.h"
#include "system.h"

extern renderer *gRenderer;
extern pixel_renderer *pRenderer;

void render_system_init(plaza *p, ecs_system *s);
void render_system_update(plaza *p, ecs_system *s, float dt);
void rigidbody_system_init(plaza *p, ecs_system *s);
void rigidbody_system_update(plaza *p, ecs_system *s, float dt);
#endif

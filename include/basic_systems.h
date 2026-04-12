#ifndef __BASIC_SYSTEMS_H__
#define __BASIC_SYSTEMS_H__
#include "input.h"
#include "renderer.h"
#include "system.h"
#include "../externals/GLFW/glfw3.h"

extern renderer *gRenderer;
extern pixel_renderer *pRenderer;
extern debug_renderer *dRenderer;
extern GLFWwindow *gw;
extern grid_buffer gb;
extern b2WorldId world_id;
extern input_handler *handler;

void render_system_init(plaza *p, ecs_system *s);
void render_system_update(plaza *p, ecs_system *s, float dt);
void pixel_system_init(plaza *p, ecs_system *s);
void pixel_system_update(plaza *p, ecs_system *s, float dt);
void rigidbody_system_init(plaza *p, ecs_system *s);
void rigidbody_system_update(plaza *p, ecs_system *s, float dt);
void physics_system_init(plaza *p, ecs_system *s);
void physics_system_update(plaza *p, ecs_system *s, float dt);
void ui_system_init(plaza *p, ecs_system *s);
void ui_system_update(plaza *p, ecs_system *s, float dt);
#endif

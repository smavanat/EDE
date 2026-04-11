#ifndef __BASIC_SYSTEMS_H__
#define __BASIC_SYSTEMS_H__
#include "component.h"
#include "input.h"
#include "maths.h"
#include "plaza.h"
#include "queue.h"
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

typedef struct {
    ivector2 cursor_pos;
    grid_buffer *gbuf;
    plaza *p;
} pixel_func_args;

typedef void (*pixel_func)(pixel_func_args* args);

typedef struct {
    pixel_func func;
    pixel_func_args *args;
} pixel_op_callback;

extern queue *pixel_func_queue;
void erase_pixels_callback(pixel_func_args *args);
void add_sand_callback(pixel_func_args *args);

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

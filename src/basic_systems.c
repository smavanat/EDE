#include "../include/basic_systems.h"
#include "../include/rigidbody.h"
#include "../include/maths.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
* FUNCTIONS FOR THE RENDERING SYSTEM
*/
void render_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << SPRITE); //Add the sprite bit to the signature
}

void render_system_update(plaza *p, ecs_system *s, float dt) {
    render_begin_frame(gRenderer);
    for(size_t i = 0; i < s->archetypes->size; i++) {
        for(size_t j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) {
            sprite *sprite = get_component_from_entity(p, get_value(s->archetypes, archetype *, i)->entities[j], SPRITE);

            render_push_quad(gRenderer, sprite->coords, sprite->colours, sprite->uv, sprite->texture);
        }
    }
    render_end_frame(gRenderer);
}

/**
* FUNCTIONS FOR THE RIGIDBODY RENDERING SYSTEM
*/
void rigidbody_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << RIGIDBODY) | (1 << TRANSFORM);
}

void rigidbody_system_update(plaza *p, ecs_system *s, float dt) {
    double cursor_x, cursor_y;
    // glfwGetCursorPos(gw, &cursor_x, &cursor_y);
    world_grid *new_grid = malloc(sizeof(world_grid));
    new_grid->height = grid->height;
    new_grid->width = grid->width;
    new_grid->pixels = malloc(sizeof(pixel) * new_grid->width * new_grid->height);
    for(int i = 0; i < grid->height * grid->width; i++) {
        memcpy(new_grid->pixels[i].colour, (uint8_t[]){0,0,0,0}, sizeof(new_grid->pixels[i].colour));
        new_grid->pixels[i].parent_body = -1;
    }
    // render_begin_pixel_frame(pRenderer);
    for(size_t i = 0; i < s->archetypes->size; i++) {
        for(size_t j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) {
            rigidbody *rigidbody = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], RIGIDBODY);
            transform *t = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], TRANSFORM);

            for(int p = 0; p < rigidbody->pixel_count; p++) {
                vector2 old_pos = ivec_to_vec(rigidbody->pixel_coords[p]);
                vector2 rotated_pos = rotateAboutPoint(&old_pos, &(vector2){0,0}, t->angle, 1);
                ivector2 new_pos = (ivector2){floor(rotated_pos.x + t->position.x + 0.5), floor(rotated_pos.y + t->position.y + 0.5)};
                if(new_pos.x < new_grid->width && new_pos.x >= 0 && new_pos.y < new_grid->height && new_pos.y >= 0) {
                    memcpy(new_grid->pixels[(new_pos.y * new_grid->width) + new_pos.x].colour, rigidbody->colour, sizeof(new_grid->pixels[(new_pos.y * new_grid->width) + new_pos.x].colour));
                    new_grid->pixels[(new_pos.y * new_grid->width) + new_pos.x].parent_body = ((archetype **)s->archetypes->data)[i]->entities[j];
                }
            }
        }
    }
    if (glfwGetWindowAttrib(gw, GLFW_FOCUSED)) { //Need to do this otherwise erasure would happen even when the window wasn't open
        glfwGetCursorPos(gw, &cursor_x, &cursor_y);
        list *rb_list = list_alloc(10, sizeof(ivector2));
        list *entity_list = list_alloc(10, sizeof(int32_t));
        erasePixels(2, cursor_x * 0.1, cursor_y * 0.1, new_grid, rb_list);
        for(int i = 0; i < rb_list->size; i++) {
            uint32_t grid_pos = (get_value(rb_list, ivector2, i).y * new_grid->width) + get_value(rb_list, ivector2, i).x;
            rigidbody *rb = get_component_from_entity(p, new_grid->pixels[grid_pos].parent_body, RIGIDBODY);
            transform *t = get_component_from_entity(p, new_grid->pixels[grid_pos].parent_body, TRANSFORM);

            int index = -1;
            for(int i = 0; i < entity_list->size; i++) {
                if(get_value(entity_list, int32_t, i) == new_grid->pixels[grid_pos].parent_body){
                    index = i;
                    break;
                }
            }
            if(index == -1) {
                push_value(entity_list, int32_t, new_grid->pixels[grid_pos].parent_body);
            }

            vector2 d = {(get_value(rb_list, ivector2, i).x - t->position.x), (get_value(rb_list, ivector2, i).y - t->position.y)};
            vector2 rotated_pos = rotateAboutPoint(&d, &(vector2){0,0}, -t->angle, 1);
            ivector2 rel_pos = (ivector2){(int)floorf(rotated_pos.x + 0.5f), (int)floorf(rotated_pos.y + 0.5f)};

            for(int j = 0; j < rb->pixel_count; j++) {
                int dx = rb->pixel_coords[j].x - rel_pos.x;
                int dy = rb->pixel_coords[j].y - rel_pos.y;

                if(abs(dx) <= 1 && abs(dy) <= 1) {
                    rb->pixel_coords[j] = rb->pixel_coords[rb->pixel_count-1];
                    rb->pixel_count--;
                    break;
                }
            }
        }
        for(int i = 0; i < entity_list->size; i++) {
            // rigidbody *rb = get_component_from_entity(p, new_grid, RIGIDBODY)
            split_rigidbody(get_value(entity_list, int32_t, i), p, new_grid, world_id);
        }
    }
    free(grid->pixels);
    free(grid);
    grid = new_grid;
}

/**
* FUNCTIONS FOR THE PHYSICS SYSTEM
*/
void physics_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << COLLIDER) | (1 << TRANSFORM);
}

void physics_system_update(plaza *p, ecs_system *s, float dt) {
    for(size_t i = 0; i < s->archetypes->size; i++) {
        for(size_t j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) {
            transform *t = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], TRANSFORM);
            collider *c = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], COLLIDER);
            draw_collider(c, dRenderer, (vector4){0.0f, 0.0f, 1.0f, 1.0f});

            vector2 temp = b2Body_GetPosition(c->collider_id);
            t->angle = normalizeAngle(b2Rot_GetAngle(b2Body_GetRotation(c->collider_id)))/DEGREES_TO_RADIANS;
            t->position = (vector2){temp.x*METRES_TO_PIXELS, temp.y * METRES_TO_PIXELS};
        }
    }
}

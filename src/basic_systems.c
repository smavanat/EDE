#include "../include/basic_systems.h"
#include "../include/rigidbody.h"
#include "../include/maths.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

void render_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << SPRITE); //Add the sprite bit to the signature
}
void render_system_update(plaza *p, ecs_system *s, float dt) {
    render_begin_frame(gRenderer);
    for(int i = 0; i < s->archetypes->size; i++) {
        for(int j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) {
            sprite *sprite = get_component_from_entity(p, get_value(s->archetypes, archetype *, i)->entities[j], SPRITE);

            render_push_quad(gRenderer, sprite->coords, sprite->colours, sprite->uv, sprite->texture);
        }
    }
    render_end_frame(gRenderer);
}

void rigidbody_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << RIGIDBODY) | (1 << TRANSFORM);
}

void rigidbody_system_update(plaza *p, ecs_system *s, float dt) {
    render_begin_pixel_frame(pRenderer);
    for(int i = 0; i < s->archetypes->size; i++) {
        for(int j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) {
            rigidbody *rigidbody = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], RIGIDBODY);
            transform *t = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], TRANSFORM);
            uint32_t pos = world_to_pixel_pos(vec_to_ivec(t->position), PIXEL_SCREEN_WIDTH);

            vector2 centre = {t->position.x + rigidbody->width / 2.0f, t->position.y + rigidbody->height / 2.0f};
            vector2 corners[4] = {
                {t->position.x, t->position.y},
                {t->position.x + rigidbody->width, t->position.y},
                {t->position.x, t->position.y + rigidbody->height},
                {t->position.x + rigidbody->width, t->position.y + rigidbody->height}
            };

            float min_x = FLT_MAX, min_y = FLT_MAX;
            float max_x = -FLT_MAX, max_y = -FLT_MAX;

            for(int k = 0; k < 4; k++) {
                vector2 r = rotateAboutPoint(&corners[k], &centre, t->angle, 1);
                min_x = fminf(min_x, r.x);
                min_y = fminf(min_y, r.y);
                max_x = fmaxf(max_x, r.x);
                max_y = fmaxf(max_y, r.y);
            }

            int start_x = (int)floorf(min_x);
            int end_x   = (int)ceilf (max_x);
            int start_y = (int)floorf(min_y);
            int end_y   = (int)ceilf (max_y);

            for (int y = start_y; y <= end_y; y++) {
                for (int x = start_x; x <= end_x; x++) {
                    vector2 world_pos = {x, y};
                    vector2 old_pos = rotateAboutPoint(&world_pos, &centre, -t->angle, 1);
                    vector2 old_pos_tex = {old_pos.x - t->position.x, old_pos.y - t->position.y};
                    uint8_t colour[4] = {0, 0, 0, 0};
                    if(old_pos_tex.x >= 0 && old_pos_tex.x < rigidbody->width && old_pos_tex.y >= 0 && old_pos_tex.y < rigidbody->height) {
                        sample_pixel(old_pos_tex.x, old_pos_tex.y, rigidbody->pixels, rigidbody->width, rigidbody->height, colour);
                    }
                    uint32_t screen_pos = world_to_pixel_pos(vec_to_ivec(world_pos), PIXEL_SCREEN_WIDTH);
                    draw_pixel(pRenderer, screen_pos, colour);
                }
            }
            t->angle += 0.05f;
        }
    }
    render_end_pixel_frame(pRenderer);
}

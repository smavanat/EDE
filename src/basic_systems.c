#include "../include/basic_systems.h"
#include "../include/rigidbody.h"
// #include "../include/maths.h"
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

queue *pixel_func_queue;

/**
* FUNCTIONS FOR THE RENDERING SYSTEM
*/
void render_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << SPRITE) | (1 << TRANSFORM); //Add the sprite bit to the signature
}

void render_system_update(plaza *p, ecs_system *s, float dt) {
    render_begin_frame(gRenderer); //Start the texture renderer
    for(size_t i = 0; i < s->archetypes->size; i++) { //Get all of the archetypes in the system
        for(size_t j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) { //Get all of the entities in a given archetype
            sprite *sprite = get_component_from_entity(p, get_value(s->archetypes, archetype *, i)->entities[j], SPRITE); //Get the sprite component from that entity
            transform *t = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], TRANSFORM); //Get the transform

            //TODO: Account for rotation as well
            vector2 coords[4];
            for(int i = 0; i < 4; i++)
                coords[i] = (vector2){(t->position.x + sprite->coords[i].x) * PIXEL_SIZE, (t->position.y + sprite->coords[i].y) * PIXEL_SIZE};

            render_push_quad(gRenderer, coords, sprite->colours, sprite->uv, sprite->texture); //Push the sprite to be rendeered
        }
    }
    render_end_frame(gRenderer); //Stop the texture renderer and send off remaining polys to GPU
}

/*
 * FUNCTIONS FOR PIXEL SYSTEM
 */
void pixel_system_init(plaza *p, ecs_system *s) {
    pixel_func_queue = queue_alloc(10, sizeof(pixel_op_callback *));
    s->signature = 0;
    printf("New pixel data size: %lu\n", sizeof(pixel_data));
}

void pixel_system_update(plaza *p, ecs_system *s, float dt) {
    //First deal with any external changes (erasures etc made to the pixel grid):
    int ok;
    pixel_op_callback *cb;
    while(pixel_func_queue->size > 0) {
        dequeue(pixel_func_queue, pixel_op_callback *, cb, ok);
        if(!ok) break;
        cb->func(cb->args);
        free(cb->args);
        free(cb);
    }

    static int left = 0;
    world_grid *old_grid = gb.grids[gb.curr];
    world_grid *new_grid = gb.grids[(gb.curr+1)%2];

    //Adding a new particle every frame just for testing
    // old_grid->data[48] = 100;
    // memcpy(old_grid->pixels[48], (uint8_t[]){0xd6, 0xcd, 0x18, 0xff}, sizeof(pixel));

    clear_grid(new_grid);

    int dir = left;
    for(size_t i = old_grid->height; i > 0; i--) {
        if(dir) {
            for(size_t j = 0; j < old_grid->width; j++) {
                if(old_grid->data[((i-1) * old_grid->width) + j].type_variant > 0 && old_grid->parents[((i-1) * old_grid->width) + j] == 0)
                    update_pixel(old_grid, new_grid, ((i-1) * old_grid->width) + j, dir);
            }
        }
        else {
            for(size_t j = old_grid->width; j > 0; j--) {
                if(old_grid->data[((i-1) * old_grid->width) + j-1].type_variant > 0 && old_grid->parents[((i-1) * old_grid->width) + j-1] == 0)
                    update_pixel(old_grid, new_grid, ((i-1) * old_grid->width) + j-1, dir);
            }
        }
        dir = (dir + 1) % 2;
    }
    left = (left+1) %2;
}

/**
 * FUNCTIONS FOR THE RIGIDBODY SYSTEM
 */
void rigidbody_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << RIGIDBODY) | (1 << TRANSFORM);
}

//This function is very poorly written as I am just trying to figure out how things will work. Needs to be re-written in a cleaner format latter.
//Possibly with different parts separated out
void rigidbody_system_update(plaza *p, ecs_system *s, float dt) {
    double cursor_x = handler->mouseX, cursor_y = handler->mouseY; //Variables to hold the mouse cursor position

    //Clearing the 'backbuffer' world_grid so we can write to it
    uint8_t next = (gb.curr + 1) % 2;
    world_grid *next_grid = gb.grids[next];
    // clear_grid(next_grid);

    for(size_t i = 0; i < s->archetypes->size; i++) { //Getting all of the archetypes for this system
        for(size_t j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) { //Getting all of the entities in an archetype
            //Getting the rigidbody and transform components of an entity
            rigidbody *rigidbody = get_component_from_entity(p, get_value(s->archetypes, archetype *, i)->entities[j], RIGIDBODY);
            transform *t = get_component_from_entity(p, get_value(s->archetypes, archetype *, i)->entities[j], TRANSFORM);

            //Pre-compute sin and cos
            float c = cosf(t->rotation);
            float sn = sinf(t->rotation);

            //Pre-compute half-width and half-height of rigidbody
            float half_width = (rigidbody->width - 1) * 0.5f;
            float half_height = (rigidbody->height - 1) * 0.5f;

            //r^2 = x^2 + y^2
            float radius = ceilf(sqrtf(half_width*half_width + half_height*half_height));

            //Compute the dimensions of the bounding circle of the rigidbody's bounding box
            int min_x = (int)floorf(t->position.x - radius);
            int max_x = (int)ceilf (t->position.x + radius);
            int min_y = (int)floorf(t->position.y - radius);
            int max_y = (int)ceilf (t->position.y + radius);

            //Iterate over all of the pixels in the bounding circle around the rigidbody
            //Using reverse mapping -> reverse rotate all of the pixels in the bounding circle to see which ones would have been in
            //the unrotated rigidbody and what their position would have been, and only fill those ones in
            for(int world_y = min_y; world_y <= max_y; world_y++) {
                for(int world_x = min_x; world_x <= max_x; world_x++) {
                    if(world_x < 0 || world_x >= next_grid->width || world_y < 0 || world_y >= next_grid->height) continue;

                    // Compute world pixel center relative to object center
                    float dx = (world_x + 0.5) - t->position.x;
                    float dy = (world_y + 0.5) - t->position.y;

                    // Inverse rotate to find where the pixel would be if the rigidbody had 0 rotation
                    float local_x =  dx * c + dy * sn;
                    float local_y = -dx * sn + dy * c;

                    // Convert to rigidbody grid coordinates
                    float grid_x = local_x + half_width;
                    float grid_y = local_y + half_height;

                    //Rounding the grid coordinates to make sure its at the center and not the edge of the pixel
                    int ix = (int)floorf(grid_x + 0.5f);
                    int iy = (int)floorf(grid_y + 0.5f);

                    //If the unrotated pixel is in the rigidbody's bounding box and the mask says its not erased, colour it
                    if(ix >= 0 && ix < rigidbody->width && iy >= 0 && iy < rigidbody->height && rigidbody->mask[iy * rigidbody->width + ix]) {
                        // memcpy(next_grid->pixels[world_y * next_grid->width + world_x], rigidbody->colour, sizeof(pixel));

                        next_grid->parents[world_y * next_grid->width + world_x] = get_value(s->archetypes, archetype *, i)->entities[j];
                        next_grid->data[world_y * next_grid->width + world_x].type_variant = rigidbody->type;
                    }
                }
            }
        }
    }

    //Destroy the old grid and set the new grid as the old grid
    gb.curr = next;
}

/**
* FUNCTIONS FOR THE PHYSICS SYSTEM
*/
void physics_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << COLLIDER) | (1 << TRANSFORM);
}

void physics_system_update(plaza *p, ecs_system *s, float dt) {
    for(size_t i = 0; i < s->archetypes->size; i++) { //For every archetype assigned to this system
        for(size_t j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) { //For every entity in that archetype
            transform *t = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], TRANSFORM); //Get the transform
            collider *c = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], COLLIDER); //Get the collider
            draw_collider(c, dRenderer, (vector4){0.0f, 0.0f, 1.0f, 1.0f}); //Draw the collider for debug purposes

            //Set the transform position and rotation to be the collider position and rotation
            vector2 temp = b2Body_GetPosition(c->collider_id);
            t->rotation = normalise_angle(b2Rot_GetAngle(b2Body_GetRotation(c->collider_id)));
            t->position = (vector2){temp.x*METRES_TO_PIXELS, temp.y * METRES_TO_PIXELS};
        }
    }
}

/**
 * FUNCTIONS FOR THE UI SYSTEM:
 */
void ui_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << BUTTON) | (1 << TRANSFORM) | (1 << SPRITE);
}

void ui_system_update(plaza *p, ecs_system *s, float dt) {
    if(glfwGetWindowAttrib(gw, GLFW_FOCUSED)) {
        for(size_t i = 0; i < s->archetypes->size; i++) { //For every archetype assigned to this system
            for(size_t j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) { //For every entity in that archetype
                transform *t = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], TRANSFORM); //Get the transform
                button *b = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], BUTTON); //Get the transform
                int pixel_x = handler->mouseX / PIXEL_SIZE;
                int pixel_y = handler->mouseY / PIXEL_SIZE;

                if(pixel_x >= t->position.x && pixel_x < t->position.x + b->bounds.x
                    && pixel_y >= t->position.y && pixel_y < t->position.y + b->bounds.y
                    && handler->key_status[MOUSE_BUTTON_LEFT] == KEY_JUST_PRESSED) {
                    printf("Pressed a button\n");
                    b->cb(b->cb_args);
                }
            }
        }
    }
}

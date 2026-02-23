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
    render_begin_frame(gRenderer); //Start the texture renderer
    for(size_t i = 0; i < s->archetypes->size; i++) { //Get all of the archetypes in the system
        for(size_t j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) { //Get all of the entities in a given archetype
            sprite *sprite = get_component_from_entity(p, get_value(s->archetypes, archetype *, i)->entities[j], SPRITE); //Get the sprite component from that entity
            //Surely we should also get the transform here and add it to the sprite coords to accurately reflect movement?
            render_push_quad(gRenderer, sprite->coords, sprite->colours, sprite->uv, sprite->texture); //Push the sprite to be rendeered
        }
    }
    render_end_frame(gRenderer); //Stop the texture renderer and send off remaining polys to GPU
}

/**
* FUNCTIONS FOR THE RIGIDBODY RENDERING SYSTEM
*/
void rigidbody_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << RIGIDBODY) | (1 << TRANSFORM);
}

//This function is very poorly written as I am just trying to figure out how things will work. Needs to be re-written in a cleaner format latter.
//Possibly with different parts separated out
void rigidbody_system_update(plaza *p, ecs_system *s, float dt) {
    double cursor_x, cursor_y; //Variables to hold the mouse cursor position

    //Make a new temporary grid to make all updates on (will then swap grid buffers at the end of the function to update the grid)
    world_grid *new_grid = malloc(sizeof(world_grid));
    new_grid->height = grid->height;
    new_grid->width = grid->width;
    new_grid->pixels = malloc(sizeof(pixel) * new_grid->width * new_grid->height);

    //Set everything in the new grid to be blank to start with
    for(int i = 0; i < grid->height * grid->width; i++) {
        memcpy(new_grid->pixels[i].colour, (uint8_t[]){0,0,0,0}, sizeof(new_grid->pixels[i].colour));
        new_grid->pixels[i].parent_body = -1;
    }

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
                    if(world_x < 0 || world_x >= new_grid->width || world_y < 0 || world_y >= new_grid->height) continue;

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
                        memcpy(new_grid->pixels[world_y * new_grid->width + world_x].colour, rigidbody->colour, sizeof(new_grid->pixels[world_y * new_grid->width + world_x].colour));

                        new_grid->pixels[world_y * new_grid->width + world_x].parent_body = get_value(s->archetypes, archetype *, i)->entities[j];
                    }
                }
            }
        }
    }

    //Do pixel erasure after updating all of the grid pixel data
    if (glfwGetWindowAttrib(gw, GLFW_FOCUSED)) { //Only erase when the window is in focus.
        glfwGetCursorPos(gw, &cursor_x, &cursor_y); //Get the mouse position

        list *point_list = list_alloc(10, sizeof(ivector2)); //List to store coordinates of points which have been erased
        list *entity_list = list_alloc(10, sizeof(int32_t)); //List to store all of the parent entities of these rigidbodies (to prevent double rigidbody processing)
        erase_pixels(2, cursor_x * 1/(float)PIXEL_SIZE, cursor_y * 1/(float)PIXEL_SIZE, new_grid, point_list); //Erase pixels at the current cursor position

        if(point_list->size > 0)
            printf("========= NEW ERASURE ========\n");
        for(int i = 0; i < point_list->size; i++) { //For each erased point which has a rigidbody parent
            ivector2 point = get_value(point_list, ivector2, i);
            uint32_t grid_pos = (point.y * new_grid->width) + point.x; //Get its grid index

            //Checking if we have already processed a particular rigidbody by checking if we have stored the id of its parent entity
            int index = -1;
            for(int i = 0; i < entity_list->size; i++) {
                if(get_value(entity_list, int32_t, i) == new_grid->pixels[grid_pos].parent_body){
                    index = i;
                    break;
                }
            }
            //If not, mark it as processed
            if(index == -1) {
                push_value(entity_list, int32_t, new_grid->pixels[grid_pos].parent_body);
            }

            rigidbody *rb = get_component_from_entity(p, new_grid->pixels[grid_pos].parent_body, RIGIDBODY); //Get a reference to the actual rigidbody struct
            transform *t = get_component_from_entity(p, new_grid->pixels[grid_pos].parent_body, TRANSFORM); //And to its corresponding transform

            //Get the relative position of the pixel to the rigidbody that is stored in the rigidbody struct
            vector2 d = {(point.x + 0.5) - t->position.x, (point.y + 0.5f) - t->position.y};
            vector2 rotated_pos = rotate_about_point(&d, &(vector2){0,0}, -t->rotation, 1);

            //Computign half-lengths of this rigidbody
            float half_width = (rb->width - 1) * 0.5f;
            float half_height = (rb->height - 1) * 0.5f;

            //Get the pixel's position in the rigidbody
            int ix = (int)floorf(rotated_pos.x + half_width + 0.5f);
            int iy = (int)floorf(rotated_pos.y + half_height + 0.5f);

            //If the unrotated pixel is in the rigidbody's bounding box and the mask says its not erased, colour it
            if(ix >= 0 && ix < rb->width && iy >= 0 && iy < rb->height && rb->mask[iy * rb->width + ix]) {
                //Clear mask. Must be done before removing pixel
                rb->mask[iy * rb->width + ix] = 0;

                for(int j = 0; j < rb->pixel_count; j++) {
                    //Pixel coords relative to rigidbody center
                    vector2 rel = rb->pixel_coords[j];

                    //Rotate to world space
                    vector2 world_pos = rotate_about_point(&rel, &(vector2){0,0}, t->rotation, 1);
                    world_pos.x += t->position.x;
                    world_pos.y += t->position.y;

                    //Compare to the erased pixel using a small epsilon
                    float dx = world_pos.x - ((float)point.x + 0.5f);
                    float dy = world_pos.y - ((float)point.y + 0.5f);

                    if(dx*dx + dy*dy < 0.25f) { //within half a pixel
                        rb->pixel_coords[j] = rb->pixel_coords[rb->pixel_count - 1]; //remove pixel by swapping with last index
                        rb->pixel_count--; //Reduce pixel count
                        break;
                    }
                }
            }
            new_grid->pixels[grid_pos].parent_body = -1; //Need to actually set the pixel to have no parent otherwise bfs will be wierd
        }
        //Split all processed rigidbodies
        for(int i = 0; i < entity_list->size; i++) {
            printf("================ NEW SPLIT ==============\n");
            split_rigidbody(get_value(entity_list, int32_t, i), p, new_grid, world_id);
        }
    }
    //Destroy the old grid and set the new grid as the old grid
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
    for(size_t i = 0; i < s->archetypes->size; i++) { //For every archetype assigned to this system
        for(size_t j = 0; j < get_value(s->archetypes, archetype *, i)->size; j++) { //For every entity in that archetype
            transform *t = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], TRANSFORM); //Get the transform
            collider *c = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], COLLIDER); //Get the collider
            // if(b2Body_GetType(c->collider_id) == b2_dynamicBody) {
            //
            // }
            draw_collider(c, dRenderer, (vector4){0.0f, 0.0f, 1.0f, 1.0f}); //Draw the collider for debug purposes

            //Set the transform position and rotation to be the collider position and rotation
            vector2 temp = b2Body_GetPosition(c->collider_id);
            t->rotation = normalise_angle(b2Rot_GetAngle(b2Body_GetRotation(c->collider_id)));
            t->position = (vector2){temp.x*METRES_TO_PIXELS, temp.y * METRES_TO_PIXELS};
        }
    }
}

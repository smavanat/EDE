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
            rigidbody *rigidbody = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], RIGIDBODY);
            transform *t = get_component_from_entity(p, ((archetype **)s->archetypes->data)[i]->entities[j], TRANSFORM);

            //Iterate over every pixel associated with a ridbody
            for(int p = 0; p < rigidbody->pixel_count; p++) {
                vector2 old_pos = ivec_to_vec(rigidbody->pixel_coords[p]); //Get its position in the previous frame
                vector2 rotated_pos = rotate_about_point(&old_pos, &(vector2){0,0}, t->rotation, 1); //Update its position based on the transform rotation
                ivector2 new_pos = (ivector2){floor(rotated_pos.x + t->position.x + 0.5), floor(rotated_pos.y + t->position.y + 0.5)}; //Update its position based on the transform position
                if(new_pos.x < new_grid->width && new_pos.x >= 0 && new_pos.y < new_grid->height && new_pos.y >= 0) { //If the new position is in the visible region of the grid
                    //Copy over the pixel data to the new position
                    memcpy(new_grid->pixels[(new_pos.y * new_grid->width) + new_pos.x].colour, rigidbody->colour, sizeof(new_grid->pixels[(new_pos.y * new_grid->width) + new_pos.x].colour));
                    new_grid->pixels[(new_pos.y * new_grid->width) + new_pos.x].parent_body = ((archetype **)s->archetypes->data)[i]->entities[j];
                }
            }
        }
    }
    //Do pixel erasure after updating all of the grid pixel data

    //Only erase when the window is in focus. Need to do this otherwise erasure would happen even when the window wasn't open
    if (glfwGetWindowAttrib(gw, GLFW_FOCUSED)) {
        glfwGetCursorPos(gw, &cursor_x, &cursor_y); //Get the mouse position

        list *rb_list = list_alloc(10, sizeof(ivector2)); //List to store coordinates of points which have been erased
        list *entity_list = list_alloc(10, sizeof(int32_t)); //List to store all of the parent entities of these rigidbodies (to prevent double rigidbody processing)
        erasePixels(2, cursor_x * 0.1, cursor_y * 0.1, new_grid, rb_list); //Erase pixels at the current cursor position

        for(int i = 0; i < rb_list->size; i++) { //For each rigidbody whose pixels have been erased
            uint32_t grid_pos = (get_value(rb_list, ivector2, i).y * new_grid->width) + get_value(rb_list, ivector2, i).x; //Get its grid index

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
            vector2 d = {(get_value(rb_list, ivector2, i).x - t->position.x), (get_value(rb_list, ivector2, i).y - t->position.y)};
            vector2 rotated_pos = rotate_about_point(&d, &(vector2){0,0}, -t->rotation, 1);
            ivector2 rel_pos = (ivector2){(int)floorf(rotated_pos.x + 0.5f), (int)floorf(rotated_pos.y + 0.5f)};

            //Check if that relative position is stored in the rigidbody struct
            for(int j = 0; j < rb->pixel_count; j++) {
                int dx = rb->pixel_coords[j].x - rel_pos.x;
                int dy = rb->pixel_coords[j].y - rel_pos.y;

                //If it is, remove it
                if(abs(dx) <= 1 && abs(dy) <= 1) {
                    rb->pixel_coords[j] = rb->pixel_coords[rb->pixel_count-1];
                    rb->pixel_count--;
                    break;
                }
            }
            new_grid->pixels[grid_pos].parent_body = -1; //Need to actually set the pixel to have no parent otherwise bfs will be wierd
        }
        //Split all processed rigidbodies
        for(int i = 0; i < entity_list->size; i++) {
            printf("================ NEW FRAME ==============\n");
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
            draw_collider(c, dRenderer, (vector4){0.0f, 0.0f, 1.0f, 1.0f}); //Draw the collider for debug purposes

            //Set the transform position and rotation to be the collider position and rotation
            vector2 temp = b2Body_GetPosition(c->collider_id);
            t->rotation = normalise_angle(b2Rot_GetAngle(b2Body_GetRotation(c->collider_id)))/DEGREES_TO_RADIANS;
            t->position = (vector2){temp.x*METRES_TO_PIXELS, temp.y * METRES_TO_PIXELS};
        }
    }
}

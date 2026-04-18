#include "../include/pixel_sim.h"
#include "../externals/glad/glad.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/list.h"
#include "../include/input.h"
#include "../include/rigidbody.h"
#include "../include/shader.h"

pixel variant_colours[NUM_PIXEL_TYPES] = {
    {0x00, 0x00, 0x00, 0x00}, //NONE
    {0xd6, 0xcd, 0x18, 0xff}, //SAND
    {0x34, 0x34, 0xeb, 0xff}, //WATER
    {0x6e, 0x31, 0x0d, 0xff}, //WOOD
    {0x69, 0x67, 0x65, 0xff}, //STONE
};

GLuint test;
shader sh;
shader sh_2;
GLint brushPosLoc;
GLint radiusLoc;
GLuint vao;

void initialise_gpu_sim(void) {
    glCreateTextures(GL_TEXTURE_2D, 1, &test);
    glTextureStorage2D(test, 1, GL_RGBA32F, PIXEL_SCREEN_WIDTH, PIXEL_SCREEN_HEIGHT);
    glBindImageTexture(0, test, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glTextureParameteri(test, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(test, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    sh = load_shader((shader_data[1]){"../data/shaders/colour.glsl", GL_COMPUTE_SHADER}, 1);
    sh_2 = load_shader((shader_data[2]){(shader_data){"../data/shaders/colour_vert.vert", GL_VERTEX_SHADER}, (shader_data){"../data/shaders/colour_frag.frag", GL_FRAGMENT_SHADER}}, 2);
    brushPosLoc = glGetUniformLocation(sh, "brushPos");
    radiusLoc = glGetUniformLocation(sh, "radius");
}

void update_gpu_sim(void) {
    glUseProgram(sh);
    glBindImageTexture(0, test, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    if(pixel_func_queue->size > 0) {
        int ok;
        pixel_op_callback *cb;
        dequeue(pixel_func_queue, pixel_op_callback *, cb, ok);
        if(cb->func == &add_pixel_callback) {
            pixel_func_args *args = cb->args;
            add_pixel_func_args *ea = (add_pixel_func_args *)args->extra_data;
            int scale = ea->scale;
            int x = args->cursor_pos.x;
            int y = args->cursor_pos.y;

            int texY = PIXEL_SCREEN_HEIGHT- y - 1;

            glUniform2f(brushPosLoc, (float)x, (float) texY);
            glUniform1f(radiusLoc, (float)scale);
        }
    }
    else {
            glUniform2f(brushPosLoc, 0.0f, 0.0f);
            glUniform1f(radiusLoc, 0.0f);
    }

    glDispatchCompute((PIXEL_SCREEN_WIDTH + 7) / 8, (PIXEL_SCREEN_HEIGHT + 7) / 8, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    // Go back to default framebuffer (screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Set viewport to window size
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Use render shader
    glUseProgram(sh_2);

    // Bind texture for sampling
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, test);

    // Tell shader texture unit
    glUniform1i(glGetUniformLocation(sh_2, "screenTex"), 0);

    // Draw fullscreen quad
    glBindBuffer(GL_VERTEX_ARRAY, vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void update_pixel(world_grid *og, world_grid *ng, size_t index, int dir) {
    size_t new_index = index;
    size_t bottom = index + og->width;
    size_t left = index + og->width-1;
    size_t right = index + og->width+1;

    if((index / og->width) < og->height - 1) {
        if(og->data[bottom].type_variant == 0 && ng->data[bottom].type_variant == 0) {
            new_index = bottom;
        }
        else {
            if(dir) {
                if(index % og->width != og->width-1 && og->data[right].type_variant == 0 && ng->data[right].type_variant == 0) {
                    new_index = right;
                }
                else if(index % og->width != 0 && og->data[left].type_variant == 0 && ng->data[left].type_variant == 0) {
                    new_index = left;
                }
            }
            else {
                if(index % og->width != 0 && og->data[left].type_variant == 0 && ng->data[left].type_variant == 0) { new_index = left;
                }
                else if(index % og->width != og->width-1 && og->data[right].type_variant == 0 && ng->data[right].type_variant == 0) {
                    new_index = right;
                }
            }
        }
    }

    //So we don't overwrite existing cells
    if(ng->data[new_index].type_variant == 0) {
        ng->data[new_index].type_variant = PIXEL_SAND;
    }
}

/**
 * Erases pixels in a square area. Used for testing the pixel destruction system
 * @param radius the half-width of the square erasure area
 * @param x the x-coordinate of the centre of the erasing square
 * @param y the y-coordinate of the centre of the erasing square
 * @param grid the world grid that erasure is occuring on
 * @param rbs a pointer to a list to store the coordintes of erased pixels that are part of a rigidbody
 */
void erase_pixels(int radius, int x, int y, world_grid *grid, list *rb_pts) {
    if (radius > 0) { //If the square is not one pixel in side length
        for (int h = 0; h < radius * 2; h++) {
            for (int w = 0; w < radius * 2; w++) {
                int dx = radius - w; // horizontal offset
                int dy = radius - h; // vertical offset
                //If the offset is in the grid and circle boundaries
                if((dx * dx + dy * dy) < (radius * radius) && (x + dx < grid->width) && (x + dx > -1) && (y + dy < grid->height) && (y + dy > -1)) {
                    //Set the pixel at this grid position to be dataless as its erased
                    grid->data[(y + dy) * grid->width + (x + dx)].type_variant = 0;
                    //If the pixel has a rigidbody parent, add it to rb_pts
                    if(grid->parents[(y + dy) * grid->width + (x + dx)] > 0) {
                        ivector2 new_pos = (ivector2){x+dx, y+dy};
                        push_value(rb_pts, ivector2, new_pos);
                    }
                }
            }
        }
    }
    else { //The erasure square is only one pixel in size
        //Set the pixel at this grid position to be dataless
        grid->data[(y  * grid->width) + x ].type_variant = 0;
        //If the pixel has a rigidbody parent, add it to rb_pts
        if(grid->parents[(y * grid->width) + x] > 0) {
            ivector2 new_pos = (ivector2){x, y};
            push_value(rb_pts, ivector2, new_pos);
        }
    }
}


void erase_pixels_callback(pixel_func_args *args) {
    //Do pixel erasure after updating all of the grid pixel data
    double cursor_x = handler->mouseX;
    double cursor_y = handler->mouseY;
    world_grid *og = args->gbuf->grids[args->gbuf->curr];
    add_pixel_func_args *ea = args->extra_data;
    plaza *p = (plaza *) ea->p;
    // plaza *p = (plaza *)args->extra_data;

    list *point_list = list_alloc(10, sizeof(ivector2)); //List to store coordinates of points which have been erased
    list *entity_list = list_alloc(10, sizeof(int32_t)); //List to store all of the parent entities of these rigidbodies (to prevent double rigidbody processing)
    erase_pixels(ea->scale, cursor_x * 1/(float)PIXEL_SIZE, cursor_y * 1/(float)PIXEL_SIZE, og, point_list); //Erase pixels at the current cursor position

    if(point_list->size > 0)
        printf("========= NEW ERASURE ========\n");
    for(int i = 0; i < point_list->size; i++) { //For each erased point which has a rigidbody parent
        ivector2 point = get_value(point_list, ivector2, i);
        uint32_t grid_pos = (point.y * og->width) + point.x; //Get its grid index

        //Checking if we have already processed a particular rigidbody by checking if we have stored the id of its parent entity
        int index = -1;
        for(int i = 0; i < entity_list->size; i++) {
            if(get_value(entity_list, int32_t, i) == og->parents[grid_pos]){
                index = i;
                break;
            }
        }
        //If not, mark it as processed
        if(index == -1) {
            push_value(entity_list, int32_t, og->parents[grid_pos]);
        }

        rigidbody *rb = get_component_from_entity(p, og->parents[grid_pos], RIGIDBODY); //Get a reference to the actual rigidbody struct
        transform *t = get_component_from_entity(p, og->parents[grid_pos], TRANSFORM); //And to its corresponding transform

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
        og->parents[grid_pos] = -1; //Need to actually set the pixel to have no parent otherwise bfs will be wierd
    }
    //Split all processed rigidbodies
    for(int i = 0; i < entity_list->size; i++) {
        printf("================ NEW SPLIT ==============\n");
        split_rigidbody(get_value(entity_list, int32_t, i), p, og, world_id);
    }

    free(ea);
    free_list(point_list);
    free_list(entity_list);
}

void add_pixel_callback(pixel_func_args *args) {
    world_grid *og = args->gbuf->grids[args->gbuf->curr];
    add_pixel_func_args *ea = (add_pixel_func_args *)args->extra_data;
    int scale = ea->scale;
    int x = args->cursor_pos.x;
    int y = args->cursor_pos.y;

    if(scale > 1) {
        for(int h = 0; h < scale; h++) {
            for(int w = 0; w < scale; w++) {
                int dx = scale - w; // horizontal offset
                int dy = scale - h; // vertical offset
                int idx = (y + dy) * og->width + (x + dx);
                //If the offset is in the grid boundary and the cell at this offset is not occupied
                if((dx * dx + dy * dy) < (scale * scale) && (x + dx < og->width)
                    && (x + dx > -1)  && (y + dy < og->height) && (y + dy > -1)
                    && og->data[idx].type_variant == PIXEL_NONE && og->parents[idx] == 0) {
                    //Set the pixel at this grid position to be the variant specified
                    og->data[(y + dy) * og->width + (x + dx)].type_variant = ea->type_variant;
                }
            }
        }
    }
    else {
        int idx = args->cursor_pos.y * og->width + args->cursor_pos.x;

        if(og->data[idx].type_variant == PIXEL_NONE && og->parents[idx] == 0) {
            og->data[idx].type_variant = ea->type_variant;
        }
    }
    free(ea);
}


/**
 * Initialises a world_grid to be blank
 * @param width the width of the grid
 * @param height the height of the grid
 * @return a pointer to the created world_grid on the heap
 */
world_grid *initialise_grid(uint32_t width, uint32_t height) {
    world_grid *grid = malloc(sizeof(world_grid));
    grid->height = height;
    grid->width = width;
    grid->pixels = calloc(width * height, sizeof(pixel)); //All pixels are initially 0
    grid->parents = calloc(width * height, sizeof(uint32_t));
    grid->data = calloc(width * height, sizeof(pixel_data));
    return grid;
}

/**
 * Clears the pixels and parents buffers of a world_grid, setting the former to 0 and the latter to -1
 * @param grid the grid to clear
 */
void clear_grid(world_grid *grid) {
    memset(grid->pixels, 0, sizeof(uint8_t) * grid->width * grid->height * 4);
    memset(grid->parents, 0, sizeof(uint32_t) * grid->width * grid->height);
    memset(grid->data, 0, sizeof(pixel_data) * grid->width * grid->height);
}

/**
 * Frees a world_grid alongside its pixels and parents buffers
 * @param grid the grid to free
 */
void free_grid(world_grid *grid) {
    free(grid->parents);
    free(grid->pixels);
    free(grid);
}

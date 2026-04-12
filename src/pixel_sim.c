#include "../include/pixel_sim.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/list.h"
#include "../include/input.h"
#include "../include/rigidbody.h"

uint32_t pixel_type_data[] = {
    0b10000000000000000000000000000000, //SAND
    0b01000000000000000000000000000000, //WOOD
    0b00100000000000000000000000000000  //STONE
};

uint16_t get_pixel_type(pixel_data pd) {
    //         12345678901234567890123456789012
    return ((0b11111111111100000000000000000000 & pd) >> 20);
}
uint8_t get_pixel_variant(pixel_data pd) {
    //         12345678901234567890123456789012
    return ((0b00000000000011110000000000000000 & pd) >> 16);
}
uint8_t get_pixel_velocityX(pixel_data pd) {
    //         12345678901234567890123456789012
    return ((0b00000000000000001111100000000000 & pd) >> 11);
}
uint8_t get_pixel_velocityY(pixel_data pd) {
    //         12345678901234567890123456789012
    return ((0b00000000000000000000011111000000 & pd) >> 6);
}
uint8_t get_pixel_health(pixel_data pd) {
    //         12345678901234567890123456789012
    return ((0b00000000000000000000000000111111 & pd));
}

void overwrite_bits(uint32_t *val, uint32_t new_bits, int start_bit, int n_bits) {
    if(n_bits <= 0 || n_bits > 32 || start_bit < 0 || start_bit > 31) return;

    if(n_bits + start_bit > 32)n_bits = 32 - start_bit;

    uint32_t mask = ((1U << n_bits)-1) << start_bit;
    *val = (*val & ~mask) | ((new_bits & ((1U << n_bits) - 1)) << start_bit);
}

void set_pixel_type(pixel_data *pd, uint16_t new_type) {
    overwrite_bits(pd, new_type, 0, 12);
}
void set_pixel_variant(pixel_data *pd, uint8_t new_var) {
    overwrite_bits(pd, new_var, 12, 4);
}
void set_pixel_velocityX(pixel_data *pd, uint8_t new_vx) {
    overwrite_bits(pd, new_vx, 16, 5);
}
void set_pixel_velocityY(pixel_data *pd, uint8_t new_vy) {
    overwrite_bits(pd, new_vy, 21, 5);
}
void set_pixel_health(pixel_data *pd, uint8_t new_health) {
    overwrite_bits(pd, new_health, 26, 6);
}

void update_pixel(world_grid *og, world_grid *ng, size_t index, int dir) {
    size_t new_index = index;
    size_t bottom = index + og->width;
    size_t left = index + og->width-1;
    size_t right = index + og->width+1;

    if((index / og->width) < og->height - 1) {
        if(og->data[bottom] == 0 && ng->data[bottom] == 0) {
            new_index = bottom;
        }
        else {
            if(dir) {
                if(index % og->width != og->width-1 && og->data[right] == 0 && ng->data[right] == 0) {
                    new_index = right;
                }
                else if(index % og->width != 0 && og->data[left] == 0 && ng->data[left] == 0) {
                    new_index = left;
                }
            }
            else {
                if(index % og->width != 0 && og->data[left] == 0 && ng->data[left] == 0) { new_index = left;
                }
                else if(index % og->width != og->width-1 && og->data[right] == 0 && ng->data[right] == 0) {
                    new_index = right;
                }
            }
        }
    }

    //So we don't overwrite existing cells
    if(ng->data[new_index] == 0) {
        ng->data[new_index] = 1;
        memcpy(ng->pixels[new_index], (uint8_t[]){0xd6, 0xcd, 0x18, 0xff}, sizeof(pixel));
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
                if((x + dx < grid->width) && (x + dx > -1) && (y + dy < grid->height) && (y + dy > -1)) { //If the offset is in the grid boundary
                    //Set the pixel at this grid position to be dataless as its erased
                    grid->data[(y + dy) * grid->width + (x + dx)] = 0;
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
        grid->data[(y  * grid->width) + x ] = 0;
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

    list *point_list = list_alloc(10, sizeof(ivector2)); //List to store coordinates of points which have been erased
    list *entity_list = list_alloc(10, sizeof(int32_t)); //List to store all of the parent entities of these rigidbodies (to prevent double rigidbody processing)
    erase_pixels(2, cursor_x * 1/(float)PIXEL_SIZE, cursor_y * 1/(float)PIXEL_SIZE, og, point_list); //Erase pixels at the current cursor position

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

        rigidbody *rb = get_component_from_entity(args->p, og->parents[grid_pos], RIGIDBODY); //Get a reference to the actual rigidbody struct
        transform *t = get_component_from_entity(args->p, og->parents[grid_pos], TRANSFORM); //And to its corresponding transform

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
        split_rigidbody(get_value(entity_list, int32_t, i), args->p, og, world_id);
    }
}

void add_sand_callback(pixel_func_args *args) {
    world_grid *og = args->gbuf->grids[args->gbuf->curr];

    int idx = args->cursor_pos.y * og->width + args->cursor_pos.x;

    if(og->data[idx] == 0 && og->parents[idx] == 0) {
        og->data[idx] = 2;
    }
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

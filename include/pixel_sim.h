#ifndef __PIXEL_SIM_H__
#define __PIXEL_SIM_H__
#include <stdint.h>
#include <stddef.h>
#include "maths.h"
#include "plaza.h"
#include "queue.h"

#define PIXEL_SIZE 10
//Layout of the data in the 4 bytes of pixel_data
//+------------+---------+-----------+-----------+--------+
//|    Type    | Variant | VelocityX | VelocityY | Health |
//|000000000000|  0000   |   00000   |   00000   | 000000 |
//+------------+---------+-----------+-----------+--------+
typedef uint32_t pixel_data;

// //An individual pixel
typedef uint8_t pixel[4];

typedef enum {
    SAND,
    WATER,
    WOOD,
    STONE,
    NUM_PIXEL_TYPES
} pixel_types;

extern uint32_t pixel_type_data[];

typedef struct {
    uint16_t width;
    uint16_t height;
    pixel *pixels;
    uint32_t *parents; //TODO: Check if we can change the type to store less data
    pixel_data *data;

} world_grid;

/**
 * Initialises a world_grid to be blank
 * @param width the width of the grid
 * @param height the height of the grid
 * @return a pointer to the created world_grid on the heap
 */
world_grid *initialise_grid(uint32_t width, uint32_t height);
/**
 * Clears the pixels and parents buffers of a world_grid, setting the former to 0 and the latter to -1
 * @param grid the grid to clear
 */
void clear_grid(world_grid *grid);
/**
 * Frees a world_grid alongside its pixels and parents buffers
 * @param grid the grid to free
 */
void free_grid(world_grid *grid);

/**
 * Holds the two grids that we swap between and their current index
 */
typedef struct {
    uint8_t curr;
    world_grid *grids[2];
} grid_buffer ;

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

extern b2WorldId world_id;

uint16_t get_pixel_type(pixel_data pd);
uint8_t get_pixel_variant(pixel_data pd);
uint8_t get_pixel_velocityX(pixel_data pd);
uint8_t get_pixel_velocityY(pixel_data pd);
uint8_t get_pixel_health(pixel_data pd);

void set_pixel_type(pixel_data *pd, uint16_t new_type);
void set_pixel_variant(pixel_data *pd, uint8_t new_var);
void set_pixel_velocityX(pixel_data *pd, uint8_t new_vx);
void set_pixel_velocityY(pixel_data *pd, uint8_t new_vy);
void set_pixel_health(pixel_data *pd, uint8_t new_health);

void update_pixel(world_grid *og, world_grid *ng, size_t index, int dir);

#endif

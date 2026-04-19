#ifndef __PIXEL_SIM_H__
#define __PIXEL_SIM_H__
#include <stdint.h>
#include <stddef.h>
#include "maths.h"
#include "plaza.h"
#include "queue.h"

//NOTE: Have a look at this page: https://meatbatgames.com/blog/falling-sand-gpu/ for GPU shader ideas if you ever come back to it
//      This is also a good repo: https://github.com/tranma/falling-sand-game?tab=readme-ov-file, https://github.com/GelamiSalami/GPU-Falling-Sand-CA/tree/main
//      GOATED TUTORIAL by the same guy: https://blog.okkohakola.com/SandFall/SandFallIntro

//TODO: Add more pixel types: Liquids, solids only for now
//      Implement lookup table for default values
//      Figure out how to handle particle lifetimes
//      Figure out particle physics (forces, gravity, etc, maybe checkout powder toy source)
//      Figure out gases
//      Figure out how to handle things like temperature
//      Figure out conversions e.g. Lava + water = stone+steam
//      Chunk and Multithread, try margolus neighbourhoods maybe for some nice sub-chunk speeds? See this article on how Noita did it: https://80.lv/articles/noita-a-game-based-on-falling-sand-simulation
#define PIXEL_SIZE 10

#define CANVAS_SIZE_X 160
#define CANVAS_SIZE_Y 120
#define LOCAL_SIZE_X 32
#define LOCAL_SIZE_Y 32
#define NUM_WORK_GROUPS_X (CANVAS_SIZE_X + LOCAL_SIZE_X - 1) / LOCAL_SIZE_X
#define NUM_WORK_GROUPS_Y (CANVAS_SIZE_Y + LOCAL_SIZE_Y - 1) / LOCAL_SIZE_Y

#define MATTER_EMPTY 0u
#define MATTER_SAND 1u
#define MATTER_WOOD 2u

typedef struct {
    float ra;
    float rb;
    uint16_t type_variant;
    uint16_t health;
} pixel_data;

// //An individual pixel
typedef uint8_t pixel[4];

typedef enum {
            //1234567890123456
    PIXEL_NONE  = 0b0000000000000000,
    PIXEL_SAND  = 0b0000000000000001,
    PIXEL_WATER = 0b0000000000000010,
    PIXEL_WOOD  = 0b0000000000000011,
    PIXEL_STONE = 0b0000000000000100,
    NUM_PIXEL_TYPES
} pixel_type;

extern pixel variant_colours[NUM_PIXEL_TYPES];

extern uint32_t pixel_type_data[];

typedef struct {
    uint16_t width;
    uint16_t height;
    pixel *pixels;
    uint32_t *parents; //TODO: Check if we can change the type to store less data
    // pixel_data *data;
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
    // plaza *p;
    void *extra_data;
} pixel_func_args;

typedef void (*pixel_func)(pixel_func_args* args);

typedef struct {
    pixel_func func;
    pixel_func_args *args;
} pixel_op_callback;

typedef struct {
    pixel_type type_variant;
    uint32_t scale;
    plaza *p;
} add_pixel_func_args;

typedef struct {
    //Need to pad this out to 'whole' values (vec4/u32) because of std140 alignment rules
    uint32_t cursor_pos[4];
    uint32_t variant;
    uint32_t radius;
} brush_data;

extern queue *pixel_func_queue;
void erase_pixels_callback(pixel_func_args *args);
void add_pixel_callback(pixel_func_args *args);
void initialise_gpu_sim(void);
void update_gpu_sim(void);
void render(void);

extern b2WorldId world_id;

void update_pixel(world_grid *og, world_grid *ng, size_t index, int dir);

#endif

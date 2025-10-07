#ifndef __CHUNK_H__
#define __CHUNK_H__
#include <stdint.h>
#include "../include/maths.h"
#include "../include/world.h"

#define CHUNK_GRID_WIDTH 64 //Width of a grid contained in a chunk
#define CHUNK_GRID_HEIGH 64 //Height of a grid contained in a chunk
#define TILE_SIDE_LENGTH 80 //Side length of a single grid tile in pixels
#define INITIAL_WORLD_HEIGHT 11
#define INITIAL_WORLD_WIDTH 11

extern world *w;

typedef struct {
    uint8_t tile_data[4096]; //A grid is 64x64=4096 tiles
} grid_data;

typedef struct chunk {
    // chunk *neighbours[8]; //Each chunk has 8 neighbours
    grid_data *grid; //Each chunk also stores its internal grid representation
} chunk;

typedef struct {
    int size;
    int capacity;
    int start;
    int end;
    chunk **data;
} chunk_list;

typedef struct {
    int size;
    int capacity;
    int start;
    int end;
    chunk_list **data;
} chunk_grid;

typedef struct {
    // list *chunks;
    chunk_grid *grid;
    ivector2 dimensions;
    ivector2 c_position;
} chunk_manager;

// typedef enum {
//     TOP,
//     RIGHT,
//     BOTTOM,
//     LEFT
// } addition_direction;

chunk_manager *create_chunk_manager(void);
chunk_manager *load_chunk_manager(void);
int *write_chunk_manager(chunk_manager *cm);
chunk *create_empty_chunk(chunk_manager *cm, ivector2 position);
chunk *load_chunk(chunk_manager *cm, ivector2 position);
int write_chunk(chunk_manager *cm, ivector2 position);
void create_tiles(chunk *c);

#endif

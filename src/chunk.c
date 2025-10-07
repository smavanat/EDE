#include "../include/chunk.h"
#include <stdlib.h>
#include <string.h>

//Need to add initialisations for start/end variable
chunk_manager *create_chunk_manager(void) {
    //Creating overarching chunk manager
    chunk_manager *cm = malloc(sizeof(chunk_manager));
    cm->dimensions = (ivector2){1,1};
    cm->c_position = (ivector2){0, 1};

    //Creating the struct that stores the rows of the grid
    cm->grid = malloc(sizeof(chunk_grid));
    cm->grid->size = 0;
    cm->grid->capacity = INITIAL_WORLD_HEIGHT;
    cm->grid->data = malloc(sizeof(chunk_list) * INITIAL_WORLD_HEIGHT);
    cm->grid->start = INITIAL_WORLD_HEIGHT/2;
    cm->grid->end = (INITIAL_WORLD_HEIGHT/2) + 1;

    //Creating each individual row of the grid
    for(int i = 0; i < INITIAL_WORLD_HEIGHT; i++) {
        cm->grid->data[i] = malloc(sizeof(chunk_list));
        cm->grid->data[i]->capacity = INITIAL_WORLD_WIDTH;
        cm->grid->data[i]->size = 0;
        cm->grid->data[i]->data = malloc(sizeof(chunk) * INITIAL_WORLD_WIDTH);
        cm->grid->data[i]->start = -1;
        cm->grid->data[i]->end = -1;
    }

    //Setting the center tile to be filled
    cm->grid->data[INITIAL_WORLD_HEIGHT/2]->data[INITIAL_WORLD_WIDTH/2] = malloc(sizeof(chunk));
    memset(cm->grid->data[INITIAL_WORLD_HEIGHT/2]->data[INITIAL_WORLD_WIDTH/2], 1, 4096);
    cm->grid->data[INITIAL_WORLD_HEIGHT/2]->start = INITIAL_WORLD_WIDTH/2;
    cm->grid->data[INITIAL_WORLD_HEIGHT/2]->end = (INITIAL_WORLD_WIDTH/2) + 1;
    return cm;
}

chunk_manager *load_chunk_manager(void);
int *write_chunk_manager(chunk_manager *cm);
chunk *create_empty_chunk(chunk_manager *cm, ivector2 position) {
    //Since we can only create chunks on the edges of the existing chunk grid, need to check
    //to increment height.width if x/y >= w/h
// if(position.x > )
}
chunk *load_chunk(chunk_manager *cm, ivector2 position);
int write_chunk(chunk_manager *cm, ivector2 position);

void create_tiles(chunk *c) {
    
}

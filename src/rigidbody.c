#include "../include/rigidbody.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NO_PIXEL_COLOUR (uint8_t[]){0,0,0,0}

uint32_t rigidbody_to_pixel_pos(uint32_t pos, uint32_t r_width, uint32_t w_width) {
    return ((pos/r_width) * w_width) + (pos % r_width);
}

uint32_t world_to_pixel_pos(ivector2 pos, uint32_t width) {
    return (pos.y * width) + pos.x;
}

ivector2 pixel_to_world_pos(uint32_t pos, uint32_t width) {
    return (ivector2){pos % width, pos / width};
}

//Using Bilinear interpolation to sample the pixel colour in a rotated image
void sample_pixel(float x, float y, pixel **pixel_array, uint32_t width, uint32_t height, uint8_t *ret) {
    //Get four adjacent pixels
    int x0 = floor(x);
    int x1 = ceil(x);
    int y0 = floor(y);
    int y1 = ceil(y);

    //Clamp their positions to be valid indecies
    x0 = clamp(x0, 0, width-1);
    x1 = clamp(x1, 0, width-1);
    y0 = clamp(y0, 0, height-1);
    y1 = clamp(y1, 0, height-1);

    //Get the colours of the pixels, setting them to be blank if the pixel does not exist
    uint8_t *y0x0_colour = (pixel_array[(y0 * width) + x0] == NULL) ? (uint8_t[]){0,0,0,0} : pixel_array[(y0 * width) + x0]->colour;
    uint8_t *y0x1_colour = (pixel_array[(y0 * width) + x1] == NULL) ? (uint8_t[]){0,0,0,0} : pixel_array[(y0 * width) + x1]->colour;
    uint8_t *y1x0_colour = (pixel_array[(y1 * width) + x0] == NULL) ? (uint8_t[]){0,0,0,0} : pixel_array[(y1 * width) + x0]->colour;
    uint8_t *y1x1_colour = (pixel_array[(y1 * width) + x1] == NULL) ? (uint8_t[]){0,0,0,0} : pixel_array[(y1 * width) + x1]->colour;

    float dx = x - x0;
    float dy = y - y0;

    for(int i = 0; i < 4; i++) {
        float val = y0x0_colour[i] * (1-dx) * (1-dy) + y0x1_colour[i] * dx * (1-dy) + y1x0_colour[i] * (1-dx) * dy + y1x1_colour[i] * dx * dy;
        ret[i] = (uint8_t)(val + 0.5f);
    }
}

rigidbody *create_rigidbody(uint32_t id, uint16_t width, uint16_t height, uint8_t colour[4], ivector2 startpos, world_grid *grid) {
    rigidbody *rb = malloc(sizeof(rigidbody));
    rb->height = height;
    rb->width = width;
    rb->pixel_count = width * height;
    memcpy(rb->colour, colour, sizeof(rb->colour));
    rb->pixel_coords = malloc(sizeof(ivector2) * rb->pixel_count);
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            memcpy(grid->pixels[((startpos.y + y - (height/2))*grid->width) + startpos.x + x - (width/2)].colour, colour, sizeof(grid->pixels[((startpos.y + y)*grid->width) + startpos.x + x].colour));
            grid->pixels[((startpos.y + y - (height/2))*grid->width) + startpos.x + x - (width/2)].parent_body = id;
            rb->pixel_coords[(y*width) + x ] = (ivector2){x - (width/2), y - (height/2)};
        }
    }

    return rb;
}

void erasePixels(int radius, int x, int y, world_grid *grid, list *rbs) {
    if (radius > 0) {
        for (int w = 0; w < radius * 2; w++)
        {
            for (int h = 0; h < radius * 2; h++)
            {
                int dx = radius - w; // horizontal offset
                int dy = radius - h; // vertical offset
                if ((dx * dx + dy * dy) < (radius * radius) && (x + dx < grid->width) && (x + dx > -1) && (y + dy < grid->height) && (y + dy > -1))
                {
                    memcpy(grid->pixels[(y + dy) * grid->width + (x + dx)].colour, NO_PIXEL_COLOUR, sizeof(grid->pixels[(y + dy) * grid->width + (x + dx)].colour));
                    // if(grid->pixels[(y + dy) * grid->width + (x + dx)].parent_body != -1) push_value(rbs, int8_t, grid->pixels[(y + dy) * grid->width + (x + dx)].parent_body);
                    ivector2 new_pos = (ivector2){x+dx, y+dy};
                    if(grid->pixels[(y + dy) * grid->width + (x + dx)].parent_body != -1) push_value(rbs, ivector2, new_pos);
                }
            }
        }
    }
    else {
        memcpy(grid->pixels[(y * grid->width) + x].colour, NO_PIXEL_COLOUR, sizeof(grid->pixels[(y * grid->width) + x].colour));
        ivector2 new_pos = (ivector2){x, y};
        if(grid->pixels[(y * grid->width) + x].parent_body != -1) push_value(rbs, ivector2, new_pos);
        // if(grid->pixels[(y * grid->width) + x].parent_body != -1) push_value(rbs, int8_t, grid->pixels[(y * grid->width) + x].parent_body);
    }
}

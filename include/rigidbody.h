#ifndef __RIGIDBODY_H__
#define __RIGIDBODY_H__
#include "maths.h"
#include <stdint.h>
#include "component.h"
#include "list.h"
#include "plaza.h"

uint32_t rigidbody_to_pixel_pos(uint32_t pos, uint32_t r_width, uint32_t w_width);
uint32_t world_to_pixel_pos(ivector2 pos, uint32_t width);
ivector2 pixel_to_world_pos(uint32_t pos, uint32_t width);
void sample_pixel(float x, float y, pixel **pixel_array, uint32_t width, uint32_t height, uint8_t *ret);
rigidbody *create_rigidbody(uint32_t id, uint16_t width, uint16_t height, uint8_t colour[4], ivector2 startpos, world_grid *grid);
void erasePixels(int radius, int x, int y, world_grid *grid, list *rbs);
void split_rigidbody(entity id, plaza *p, world_grid *grid, b2WorldId world_id);

#endif

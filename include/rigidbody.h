#ifndef __RIGIDBODY_H__
#define __RIGIDBODY_H__
#include "maths.h"
#include <stdint.h>
#include "../include/component.h"

uint32_t rigidbody_to_pixel_pos(uint32_t pos, uint32_t r_width, uint32_t w_width);
uint32_t world_to_pixel_pos(ivector2 pos, uint32_t width);
ivector2 pixel_to_world_pos(uint32_t pos, uint32_t width);
void sample_pixel(float x, float y, pixel **pixel_array, uint32_t width, uint32_t height, uint8_t *ret);
#endif

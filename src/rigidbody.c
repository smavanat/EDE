#include "../include/rigidbody.h"
#include <stdint.h>

uint32_t rigidbody_to_pixel_pos(uint32_t pos, uint32_t r_width, uint32_t w_width) {
    return ((pos/r_width) * w_width) + (pos % r_width);
}

uint32_t world_to_pixel_pos(ivector2 pos, uint32_t width) {
    return (pos.y * width) + pos.x;
}

ivector2 pixel_to_world_pos(uint32_t pos, uint32_t width) {
    return (ivector2){pos % width, pos / width};
}

void sample_pixel(float x, float y, pixel **pixel_array, uint32_t width, uint32_t height, uint8_t *ret) {
    int x0 = floor(x);
    int x1 = ceil(x);
    int y0 = floor(y);
    int y1 = ceil(y);
    x0 = clamp(x0, 0, width-1);
    x1 = clamp(x1, 0, width-1);
    y0 = clamp(y0, 0, height-1);
    y1 = clamp(y1, 0, height-1);
    float dx = x - x0;
    float dy = y - y0;
    for(int i = 0; i < 4; i++) {
        float val = pixel_array[(y0 * width) + x0]->colour[i] * (1-dx) * (1-dy) + pixel_array[(y0 * width) + x1]->colour[i] * dx * (1-dy) + pixel_array[(y1 * width) + x0]->colour[i] * (1-dx) * dy + pixel_array[(y1 * width) + x1]->colour[i] * dx * dy;
        ret[i] = (uint8_t)(val + 0.5f);
    }
}

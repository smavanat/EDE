#include "../include/pixel_sim.h"
#include <stdint.h>

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

#ifndef __PIXEL_SIM_H__
#define __PIXEL_SIM_H__
#include <stdint.h>

//Layout of the data in the 4 bytes of pixel_data
//+------------+---------+-----------+-----------+--------+
//|    Type    | Variant | VelocityX | VelocityY | Health |
//|000000000000|  0000   |   00000   |   00000   | 000000 |
//+------------+---------+-----------+-----------+--------+

typedef uint32_t pixel_data;

typedef enum {
    SAND,
    WOOD,
    STONE,
    NUM_PIXEL_TYPES
} pixel_types;

uint32_t pixel_type_data[] = {
                                0b10000000000000000000000000000000, //SAND
                                0b01000000000000000000000000000000, //WOOD
                                0b00100000000000000000000000000000  //STONE
                             };

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

#endif

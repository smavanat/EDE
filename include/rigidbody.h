#ifndef __RIGIDBODY_H__
#define __RIGIDBODY_H__
#include "maths.h"
#include <stdint.h>
#include "component.h"
#include "list.h"
#include "plaza.h"
#include "renderer.h"

extern debug_renderer *dRenderer;

uint32_t rigidbody_to_pixel_pos(uint32_t pos, uint32_t r_width, uint32_t w_width);
uint32_t world_to_pixel_pos(ivector2 pos, uint32_t width);
ivector2 pixel_to_world_pos(uint32_t pos, uint32_t width);
/**
 * Creates a brand new rigidbody component and returns a reference to it
 * @param id the id of its parent entity
 * @param width the width of the rigidbody
 * @param height the height of the rigidbody
 * @param type_variant the variant and type of the pixels included in the rigidbody (currently assume the variant of all the pixels in the rigidbody is uniform, can change later)
 * @param centre its centre in worldspace
 * @param grid a pointer to the world grid where this rigidbody is located
 * @return a pointer to the created rigidbody
 */
rigidbody *create_rigidbody(uint32_t id, uint16_t width, uint16_t height, uint16_t type_variant, vector2 centre, world_grid *grid);
/**
 * Creates a rigidbody from a set of pixel data rather than just width and height, mostly used for split or non-rectangular rigidbodies
 * @param id the id of its parent entity
 * @param width the width of the rigidbody
 * @param height the height of the rigidbody
 * @param type_variant the variant and type of the pixels included in the rigidbody (currently assume the variant of all the pixels in the rigidbody is uniform, can change later)
 * @param centre its centre in worldspace
 * @param pixel_coords a list containing the grid coordinates of the pixels that make up the rigidbody
 * @param grid a pointer to the world grid where this rigidbody is located
 * @return a pointer to the created rigidbody
 */
rigidbody *create_rigidbody_from_pixels(uint32_t id, uint16_t width, uint16_t height, uint8_t colour[4], vector2 centre, list *pixel_coords, world_grid *grid);
/**
 * Splits a 'dirty' (has erased pixels) rigidbody into new rigidbodies if necessary, or adjusts its collider if not
 * @param id the entity to which the rigidbody belongs
 * @param p a pointer to the plaza managing entities
 * @param grid a pointer to the world_grid where the rigidbody is located
 * @param world_id the id of the box2d world where the collider of the rigidbody is located
 */
void split_rigidbody(entity id, plaza *p, world_grid *grid, b2WorldId world_id);

#endif

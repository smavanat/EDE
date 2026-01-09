#ifndef __COMPONENT_H__
#define __COMPONENT_H__
#include "maths.h"
#include <stdbool.h>
#include <stdint.h>
#include "../externals/box2d/box2d.h"
#include "renderer.h"

typedef struct {
    vector2 points[3];
} triangle_polygon;

#define MAX_COMPONENTS 32
#define METRES_TO_PIXELS 50.0f
#define PIXELS_TO_METRES 1.0f / METRES_TO_PIXELS

typedef enum{
    TRANSFORM,
    SPRITE,
    COLLIDER,
    // PATHFINDING,
    RIGIDBODY,
    // PIXEL,
    NUM_COMPONENTS //Used for counting the number of components
} component_type;

typedef enum{
    BOX,
    CIRCLE,
    CAPSULE,
    POLYGON,
    NONE
} collider_type;

typedef struct {
    vector2 position;
    float zIndex;
    float angle;
} transform;

transform *create_transform(vector2 position, float zIndex, float angle);
void free_transform(void *t);

//Needs to be filled in later once OpenGL is setup
//Holds destructible sprite texture data
typedef struct {
    unsigned int texture;
    vector2 coords[4];
    vector4 colours[4];
    vector2 uv[4];
} sprite;

sprite *create_sprite(unsigned int texture, vector2 coords[4], vector4 colours[4], vector2 uv[4]);
void free_sprite(void *spr);

//Needs to be finished once box2D is setup
//Holds the b2BodyId of a box2D collider alongside what kind of collider it is
typedef struct {
    collider_type type;
    b2BodyId collider_id;
} collider;

b2BodyId create_circle_collider(vector2 center, float radius, b2WorldId worldId, b2BodyType type);
b2BodyId create_box_collider(vector2 center, int width, int height, float rotation, b2WorldId worldId, b2BodyType type);
b2BodyId create_capsule_collider(vector2 center1, vector2 center2, float rotation, float radius, b2WorldId worldId, b2BodyType type);
b2BodyId create_polygon_collider(vector2* points, int pointsSize, vector2 center, float rotation, b2WorldId worldId, b2BodyType type);
vector2 rotate_translate(vector2* vector, float angle);
void draw_collider(collider *c, debug_renderer *dRenderer, vector4 colour);
void free_collider(void *c);

//Holds pathfinding data to be used to calculate a viable path between two points which is then re-stored in this component
typedef struct {
    vector2* path; //This needs to be replaced with a proper array that holds the size once I've finished working on those
    vector2 startPos;
    vector2 endPos;
    int size; //The size of the agent that is traversing through this path
} pathfinding;
void free_pathfinding(void *p);

//The following components are for implementing the destruction system to be more like the way Noita actually does it, buy having the rigidbody be fully made up of pixels

//An individual pixel
typedef struct {
    uint8_t colour[4];
    int32_t parent_body; //The parent rigidbody if this pixel has one
} pixel;

//A rigidbody
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t colour[4];
    // pixel **pixels;
    // ivector2 *pixel_coords;
    ivector2 *pixel_coords;
    uint32_t pixel_count;
} rigidbody;

void free_rigidbody(void *rb);

typedef struct {
    uint16_t width;
    uint16_t height;
    pixel *pixels;
} world_grid;

#endif

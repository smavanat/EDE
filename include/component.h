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

//Enum for differentiating between the different component types
typedef enum{
    TRANSFORM,
    SPRITE,
    COLLIDER,
    // PATHFINDING,
    RIGIDBODY,
    // PIXEL,
    NUM_COMPONENTS //Used for counting the number of components
} component_type;

//Enum for differentiating between the different collider types
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
    float rotation;
} transform;

/**
 * Creates a new transform component
 * @param position the position of the transform
 * @param zIndex the z-index of the transform
 * @param rotation the rotation of the transform
 * @return a pointer to the created transform component
 */
transform *create_transform(vector2 position, float zIndex, float angle);
/**
 * Destroys an existing transform component
 * @param t a pointer to an existing transform component
 */
void free_transform(void *t);

//Holds OpenGL texture data
typedef struct {
    unsigned int texture;
    vector2 coords[4];
    vector4 colours[4];
    vector2 uv[4];
} sprite;

/**
 * Creates a new sprite component
 * @param texture an int representing the OpenGL texture id of the texture assigned to this sprite
 * @param coords an array representing the rectangular coordinates of this texture (assuming center is (0,0))
 * @param colours an array representing the colours of the texture
 * @param uv an array representing the OpenGL (u,v) coords of the texture
 * @return a pointer to the created sprite component
 */
sprite *create_sprite(unsigned int texture, vector2 coords[4], vector4 colours[4], vector2 uv[4]);
/**
 * Destroys an existing sprite component
 * @param spr a pointer to a sprite component
 */
void free_sprite(void *spr);

//Holds the b2BodyId of a box2D collider alongside what kind of collider it is
typedef struct {
    collider_type type;
    b2BodyId collider_id;
} collider;

/**
 * Creates a new circle collider
 * @param center the center of the circle in pixels
 * @param radius the radius of the circle in pixels
 * @param worldId the id of the box2d world we are creating the collider in
 * @param type the type (dynamic, static, etc.) of collider we should be creating
 * @return the id of the box2d body that was created
 */
b2BodyId create_circle_collider(vector2 center, float radius, b2WorldId worldId, b2BodyType type);
/**
 * Creates a new box collider
 * @param center the center of the box in pixels
 * @param width the width of the box in pixels
 * @param height the height of the box in pixels
 * @param rotation the rotation of the box in radians
 * @param worldId the id of the box2d world we are creating the collider in
 * @param type the type (dynamic, static, etc.) of collider we should be creating
 * @return the id of the box2d body that was created
 */
b2BodyId create_box_collider(vector2 center, int width, int height, float rotation, b2WorldId worldId, b2BodyType type);
/**
 * Creates a new capsule collider
 * @param center1 the first center of the capsule in pixels
 * @param center2 the second center of the capsule in pixels
 * @param radius the radius of both of the centers in pixels
 * @param rotation the rotation of the capsule in radians
 * @param worldId the id of the box2d world we are creating the collider in
 * @param type the type (dynamic, static, etc.) of collider we should be creating
 * @return the id of the box2d body that was created
 */
b2BodyId create_capsule_collider(vector2 center1, vector2 center2, float rotation, float radius, b2WorldId worldId, b2BodyType type);
/**
 * Creates a polygon collider from a set of points by triangulating them
 * @param points the set of points that make up the polygon in CCW order in pixels
 * @param points_size the number of points in the polygon
 * @param center the center that the collider should be placed at in pixels
 * @param rotation the rotation of the collider in radians
 * @param worldId the id of the box2d world we are using for the collider
 * @param type the box2d type of the collider
 * @return the box2d b2BodyId of the collider
 */
b2BodyId create_polygon_collider(vector2* points, int pointsSize, vector2 center, float rotation, b2WorldId worldId, b2BodyType type);
/**
 * Rotates a point by an angle
 * @param vector the point to rotate
 * @param angle the angle to rotate it by in radians
 * @return a new vector representing the rotated point
 */
vector2 rotate_translate(vector2* vector, float angle);
/**
 * Draws collider outlines for debugging purposes
 * @param c a pointer to the collider whose outline needs to be drawn
 * @param dRenderer a pointer to the debug_renderer doing the drawing
 * @param colour the colour of the outline
 */
void draw_collider(collider *c, debug_renderer *dRenderer, vector4 colour);
/**
 * Destroys an existing collider component
 * @param c a pointer to a collider component
 */
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

/**
 * Destroys an existing rigidbody component
 * @param rb a pointer to a rigidbody component
 */
void free_rigidbody(void *rb);

typedef struct {
    uint16_t width;
    uint16_t height;
    pixel *pixels;
} world_grid;

#endif

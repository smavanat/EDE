#ifndef __COMPONENT_H__
#define __COMPONENT_H__
#include "maths.h"
#include <stdbool.h>
#include "shader.h"

#define MAX_COMPONENTS (32)

typedef enum{
    TRANSFORM,
    SPRITE,
    TILE_SPRITE,
    COLLIDER,
    TAG,
    TILE_RECT,
    PATHFINDING,
    TERRAIN,
    BUTTON,
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

//Needs to be filled in later once OpenGL is setup
//Holds destructible sprite texture data
typedef struct {
    shader shader;
    unsigned int texture;
    unsigned int VAO;
} sprite;

//Holds indestructible sprite data from a tilemap or a texture atlas.
typedef struct {

} tile_sprite;

//Needs to be finished once box2D is setup
//Holds the b2BodyId of a box2D collider alongside what kind of collider it is
typedef struct {
    collider_type type;
} collider;

//A tag for differentiating entities from eachother
typedef struct {
    int tagId;
} tag;

typedef struct {

} tile_rect;

//Holds pathfinding data to be used to calculate a viable path between two points which is then re-stored in this component
typedef struct {
    vector2* path; //This needs to be replaced with a proper array that holds the size once I've finished working on those
    vector2 startPos;
    vector2 endPos;
    int size; //The size of the agent that is traversing through this path
} pathfinding;

//For destructible sprites
typedef struct {
    bool isTerrain;
} terrain;

#endif

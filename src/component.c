#include "../include/component.h"
#include <stdlib.h>
#include <string.h>

transform *create_transform(vector2 position, float zIndex, float angle) {
    transform *t = malloc(sizeof(transform));
    t->angle = angle;
    t->zIndex = zIndex;
    t->position = position;
    return t;
}

sprite *create_sprite(unsigned int texture, vector2 coords[4], vector4 colours[4], vector2 uv[4]) {
    sprite *spr = malloc(sizeof(sprite));
    spr->texture = texture;
    memcpy(spr->colours, colours, sizeof(spr->colours));
    memcpy(spr->coords, coords, sizeof(spr->coords));
    memcpy(spr->uv, uv, sizeof(spr->uv));

    return spr;
}

b2BodyId createCircleCollider(vector2 center, float radius, b2WorldId worldId, b2BodyType type) {
    //Default shape initialisation code
    b2BodyDef retBodyDef = b2DefaultBodyDef();
    retBodyDef.type = type;
    retBodyDef.position = (vector2){center.x * PIXELS_TO_METRES, center.y * PIXELS_TO_METRES};
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);

    //Circle creation
    b2Circle circle = {{0.0f, 0.0f}, radius*PIXELS_TO_METRES};
    b2ShapeDef sd = b2DefaultShapeDef();
    b2CreateCircleShape(retId, &sd, &circle);

    return retId;
}

b2BodyId createBoxCollider(vector2 center, int width, int height, float rotation, b2WorldId worldId, b2BodyType type) {
    //Default shape initialisation code
    b2BodyDef retBodyDef = b2DefaultBodyDef();
    retBodyDef.type = type;
    retBodyDef.position = (vector2){center.x * PIXELS_TO_METRES, center.y * PIXELS_TO_METRES};
    retBodyDef.rotation = (b2Rot){(float)cos(rotation * DEGREES_TO_RADIANS), (float)sin(rotation * DEGREES_TO_RADIANS)};
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);

    //Box creation
    b2Polygon box = b2MakeBox((width/2.0f)*PIXELS_TO_METRES, (height/2.0f)*PIXELS_TO_METRES);
    b2ShapeDef sd = b2DefaultShapeDef();
    b2CreatePolygonShape(retId, &sd, &box);

    return retId;
}

b2BodyId createCapsuleCollider(vector2 center1, vector2 center2, float radius, float rotation, b2WorldId worldId, b2BodyType type) {
    //Calculate actual center of the capsule
    vector2 center = (vector2){(center2.x - center1.x)/2.0f, (center2.y - center1.y)/2.0f};

    //Default shape initialisation code
    b2BodyDef retBodyDef = b2DefaultBodyDef();
    retBodyDef.type = type;
    retBodyDef.position = (vector2){center.x * PIXELS_TO_METRES, center.y * PIXELS_TO_METRES};
    retBodyDef.rotation = (b2Rot){(float)cos(rotation * DEGREES_TO_RADIANS), (float)sin(rotation * DEGREES_TO_RADIANS)};
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);

    //Capsule creation
    b2Capsule capsule = {{center1.x*PIXELS_TO_METRES, center1.y*PIXELS_TO_METRES}, {center2.x*PIXELS_TO_METRES, center2.y*PIXELS_TO_METRES}, radius*PIXELS_TO_METRES};
    b2ShapeDef sd = b2DefaultShapeDef();
    b2CreateCapsuleShape(retId, &sd, &capsule);

    return retId;
}

b2BodyId createPolygonCollider(vector2* points, int pointsSize, vector2 center, float rotation, b2WorldId worldId, b2BodyType type);
vector2 rotateTranslate(vector2* vector, float angle);

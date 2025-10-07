#pragma once
#ifndef MATHS_H
#define MATHS_H
#include <math.h>
#include <stdbool.h>
// #include"externals/Include/box2d/box2d.h"

extern double DEGREES_TO_RADIANS;

//Could we not just make this a typdef of b2Vec2?
// typedef b2Vec2 vector2;
typedef struct {
    float x;
    float y;
} vector2;

typedef struct {
    int x;
    int y;
} ivector2;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vector4;

typedef struct {
    float x;
    float y;
    float w;
    float h;
} frect;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} rect;

#ifdef __cplusplus
extern "C" {
#endif
//Rotates a vector2 about the z-axis and returns the result. Takes an angle in degrees or in radians, but if 
//the angle is in degrees, the inRadians parameter must be set to false;
vector2 rotate(vector2* vec, double angle, bool inRadians);

//Rotates a vector2 about a point in the z-axis and returns the result. Takes an angle in degrees or in radians, 
//but if the angle is in degrees, the inRadians parameter must be set to false.
vector2 rotateAboutPoint(vector2* point, vector2* centre, double angle, bool inRadians);

vector2 rotateAboutPoint2(vector2* point, vector2* centre, double angle, bool inRadians);

bool equals(vector2 a, vector2 b);

double normalizeAngle(double angle);

void normalise(vector2* v);

#ifdef __cplusplus
}
#endif

#endif

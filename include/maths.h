#ifndef MATHS_H
#define MATHS_H
#include <math.h>
#include <stdbool.h>
#include "../externals/box2d/box2d.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEGREES_TO_RADIANS M_PI / 180

typedef b2Vec2 vector2;
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
/**
 * Converts a vector2 to an ivector2 by casting its x and y components to ints
 * @param v the vector2 to convert
 * @return the converted vector as an ivector2
 */
ivector2 vec_to_ivec(vector2 v);
/**
 * Converts an ivector2 to an ivector2 by casting its x and y components to floats
 * @param v the ivector2 to convert
 * @return the converted ivector as a vector2
 */
vector2 ivec_to_vec(ivector2 v);
/**
 * Rotates a vector2 about the z-axis and returns the result
 * @param vec the vector2 to rotate
 * @param angle the angle to rotate around. Can be in degrees or radians
 * @param in_radians true if angle has been provided in radians. false otherwise
 * @return the rotated vector
 */
vector2 rotate(vector2* vec, double angle, bool inRadians);
/**
 * Rotates a vector2 about a point in the z-axis and returns the result
 * @param point the vector2 to rotate
 * @param centre the point to rotate about
 * @param angle the angle to rotate around. Can be in degrees or radians
 * @param in_radians true if angle has been provided in radians. false otherwise
 * @return the rotated vector
 */
vector2 rotate_about_point(vector2* point, vector2* centre, double angle, bool inRadians);
/**
 * Checks if two vectors are the same by seeing if their constituent parts are the same
 * @param a the first vector to compare
 * @param b the second vector to compare
 * @return true if they are the same, false otherwise
 */
bool equals(vector2 a, vector2 b);
/**
 * Normalises an angle between 0 and 2pi
 * @param angle the angle to normalise. Must be in radians
 * @return the normalised angle
 */
double normalise_angle(double angle);
/**
 * Normalises a vector in the range [-1, 1];
 * @param v the vector to normalise
 * @return the normalised vector
 */
vector2 normalise_vector(vector2* v);
/**
 * Clamps an integer between two values
 * @param val the integer to clamp
 * @param min_val the lower bound of the clamp
 * @param max_val the upper bound of the clamp
 * @return val clamped between min_val and max_val
 */
int clamp(int val, int min_val, int max_val);
/**
 * Returns the distance between two vectors
 * @param v1 the first vector
 * @param v2 the second vector
 * @return the absolute distance between v1 and v2
 */
float distance(vector2 *v1, vector2 *v2);

#ifdef __cplusplus
}
#endif

#endif

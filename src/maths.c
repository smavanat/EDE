#include "../include/maths.h"
#include <math.h>

/**
 * Converts a vector2 to an ivector2 by casting its x and y components to ints
 * @param v the vector2 to convert
 * @return the converted vector as an ivector2
 */
ivector2 vec_to_ivec(vector2 v) {
    return (ivector2){(int)v.x, (int)v.y};
}

/**
 * Converts an ivector2 to an ivector2 by casting its x and y components to floats
 * @param v the ivector2 to convert
 * @return the converted ivector as a vector2
 */
vector2 ivec_to_vec(ivector2 v) {
    return (vector2){(float)v.x, (float)v.y};
}

/**
 * Rotates a vector2 about the z-axis and returns the result
 * @param vec the vector2 to rotate
 * @param angle the angle to rotate around. Can be in degrees or radians
 * @param in_radians true if angle has been provided in radians. false otherwise
 * @return the rotated vector
 */
vector2 rotate(vector2* vec, double angle, bool in_radians) {
    vector2 ret = {};
    if (!in_radians) {
        angle *= DEGREES_TO_RADIANS;
    }
    ret.x = (int)(round(vec->x * cos(angle) - vec->y * sin(angle)));
    ret.y = (int)(round(vec->x * sin(angle) + vec->y * cos(angle)));
    return ret;
}

/**
 * Rotates a vector2 about a point in the z-axis and returns the result
 * @param point the vector2 to rotate
 * @param centre the point to rotate about
 * @param angle the angle to rotate around. Can be in degrees or radians
 * @param in_radians true if angle has been provided in radians. false otherwise
 * @return the rotated vector
 */
vector2 rotate_about_point(vector2* point, vector2* centre, double angle, bool in_radians) {
    vector2 ret = {};
    if (!in_radians) {
        angle *= DEGREES_TO_RADIANS;
    }
    ret.x = (centre->x + (point->x - centre->x) * cos(angle) - (point->y - centre->y) * sin(angle));
    ret.y = (centre->y + (point->x - centre->x) * sin(angle) + (point->y - centre->y) * cos(angle));
    return ret;
}

/**
 * Normalises an angle between 0 and 2pi
 * @param angle the angle to normalise. Must be in radians
 * @return the normalised angle
 */
double normalise_angle(double angle) {
    if (angle < 0) {
        return angle + 2 * (2 * acos(0.0));  // Shift negative angles into [0, 2pi]
    }
    return angle;
}

/**
 * Normalises a vector in the range [-1, 1];
 * @param v the vector to normalise
 * @return the normalised vector
 */
vector2 normalise_vector(vector2* v) {
    vector2 ret = {};
    float mag = sqrt((v->x*v->x)+(v->y*v->y));

    ret.x = v->x / mag;
    ret.y = v->y / mag;
    return ret;
}

/**
 * Checks if two vectors are the same by seeing if their constituent parts are the same
 * @param a the first vector to compare
 * @param b the second vector to compare
 * @return true if they are the same, false otherwise
 */
bool equals(vector2 a, vector2 b) {
    return fabs(a.x - b.x) < 0.001f && fabs(a.y - b.y) < 0.001f;
}

/**
 * Clamps an integer between two values
 * @param val the integer to clamp
 * @param min_val the lower bound of the clamp
 * @param max_val the upper bound of the clamp
 * @return val clamped between min_val and max_val
 */
int clamp(int val, int min_val, int max_val) {
    if(val < min_val) {
        return min_val;
    }
    if(val > max_val) {
        return max_val;
    }
    return val;
}

/**
 * Returns the distance between two vectors
 * @param v1 the first vector
 * @param v2 the second vector
 * @return the absolute distance between v1 and v2
 */
float distance(vector2 *v1, vector2 *v2) {

    float dx, dy;
    dx = v2->x - v1->x;
    dy = v2->y - v1->y;
    return (sqrtf(dx * dx + dy * dy));
}

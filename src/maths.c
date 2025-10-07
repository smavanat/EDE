#include "../include/maths.h"
#include <math.h>
double DEGREES_TO_RADIANS = (M_PI / 180);

//Rotates a vector2 about the z-axis and returns the result. Takes an angle in degrees or in radians, but if 
//the angle is in degrees, the inRadians parameter must be set to false;
vector2 rotate(vector2* vec, double angle, bool inRadians) {
	vector2 ret = {};
	if (!inRadians) {
		angle *= DEGREES_TO_RADIANS;
	}
	ret.x = (int)(round(vec->x * cos(angle) - vec->y * sin(angle)));
	ret.y = (int)(round(vec->x * sin(angle) + vec->y * cos(angle)));
	return ret;
}

//Rotates a vector2 about a point in the z-axis and returns the result. Takes an angle in degrees or in radians, 
//but if the angle is in degrees, the inRadians parameter must be set to false.
vector2 rotateAboutPoint(vector2* point, vector2* centre, double angle, bool inRadians) {
	vector2 ret = {};
	if (!inRadians) {
		angle *= DEGREES_TO_RADIANS;
	}
	ret.x = (centre->x + (point->x - centre->x) * cos(angle) - (point->y - centre->y) * sin(angle));
	ret.y = (centre->y + (point->x - centre->x) * sin(angle) + (point->y - centre->y) * cos(angle));
	return ret;
}

vector2 rotateAboutPoint2(vector2* point, vector2* centre, double angle, bool inRadians) {
	vector2 ret = {};
	angle = fmod(angle, 360);
	if (!inRadians) {
		angle *= DEGREES_TO_RADIANS;
	}
	ret.x = (centre->x + (point->x - centre->x) * cos(angle) - (point->y - centre->y) * sin(angle));
	ret.y = (centre->y + (point->x - centre->x) * sin(angle) + (point->y - centre->y) * cos(angle));
	return ret;
}

double normalizeAngle(double angle) {
	if (angle < 0) {
		return angle + 2 * (2 * acos(0.0));  // Shift negative angles into [0, 2pi]
	}
	return angle;
}

void normalise(vector2* v) {
	float mag = sqrt((v->x*v->x)+(v->y*v->y));

	v->x /= mag;
	v->y /= mag;
}

bool equals(vector2 a, vector2 b) {
	return fabs(a.x - b.x) < 0.001f && fabs(a.y - b.y) < 0.001f;
}

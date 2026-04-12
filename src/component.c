#include "../include/component.h"
#include "../include/list.h"
#include "../include/queue.h"
#include "../externals/glad/glad.h"
#include <float.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Creates a new transform component
 * @param position the position of the transform
 * @param zIndex the z-index of the transform
 * @param rotation the rotation of the transform
 * @return a pointer to the created transform component
 */
transform *create_transform(vector2 position, float zIndex, float rotation) {
    transform *t = malloc(sizeof(transform));
    t->rotation = rotation;
    t->zIndex = zIndex;
    t->position = position;
    return t;
}

/**
 * Destroys an existing transform component
 * @param t a pointer to an existing transform component
 */
void free_transform(void *t) {
    transform *tr = (transform *) t;
    tr->position = (vector2){0,0};
    tr->rotation = 0;
    tr->zIndex = 0;
}

/**
 * Creates a new sprite component
 * @param texture an int representing the OpenGL texture id of the texture assigned to this sprite
 * @param coords an array representing the rectangular coordinates of this texture (assuming center is (0,0))
 * @param colours an array representing the colours of the texture
 * @param uv an array representing the OpenGL (u,v) coords of the texture
 * @return a pointer to the created sprite component
 */
sprite *create_sprite(unsigned int texture, vector2 coords[4], vector4 colours[4], vector2 uv[4]) {
    sprite *spr = malloc(sizeof(sprite));
    spr->texture = texture;
    //Need to do memcpy as they are arrays
    memcpy(spr->colours, colours, sizeof(spr->colours));
    memcpy(spr->coords, coords, sizeof(spr->coords));
    memcpy(spr->uv, uv, sizeof(spr->uv));

    return spr;
}

/**
 * Destroys an existing sprite component
 * @param spr a pointer to a sprite component
 */
void free_sprite(void *spr) {
    sprite *s = (sprite *)spr;
    glDeleteTextures(1, &s->texture); //Need to delete the OpenGL texture
}

/**
 * Creates a new circle collider
 * @param center the center of the circle in pixels
 * @param radius the radius of the circle in pixels
 * @param worldId the id of the box2d world we are creating the collider in
 * @param type the type (dynamic, static, etc.) of collider we should be creating
 * @return the id of the box2d body that was created
 */
b2BodyId create_circle_collider(vector2 center, float radius, b2WorldId worldId, b2BodyType type) {
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
b2BodyId create_box_collider(vector2 center, int width, int height, float rotation, b2WorldId worldId, b2BodyType type) {
    //Default shape initialisation code
    b2BodyDef retBodyDef = b2DefaultBodyDef();
    retBodyDef.type = type;
    retBodyDef.position = (vector2){center.x * PIXELS_TO_METRES, center.y * PIXELS_TO_METRES};
    retBodyDef.rotation = (b2Rot){(float)cos(rotation), (float)sin(rotation)};
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);

    //Box creation
    b2Polygon box = b2MakeBox((width/2.0f)*PIXELS_TO_METRES, (height/2.0f)*PIXELS_TO_METRES); //Need to do half-widths because that's what box2d specifies for some reason
    b2ShapeDef sd = b2DefaultShapeDef();
    b2CreatePolygonShape(retId, &sd, &box);

    return retId;
}

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
b2BodyId create_capsule_collider(vector2 center1, vector2 center2, float radius, float rotation, b2WorldId worldId, b2BodyType type) {
    //Calculate actual center of the capsule
    vector2 center = (vector2){(center2.x - center1.x)/2.0f, (center2.y - center1.y)/2.0f};

    //Default shape initialisation code
    b2BodyDef retBodyDef = b2DefaultBodyDef();
    retBodyDef.type = type;
    retBodyDef.position = (vector2){center.x * PIXELS_TO_METRES, center.y * PIXELS_TO_METRES};
    retBodyDef.rotation = (b2Rot){(float)cos(rotation), (float)sin(rotation)};
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);

    //Capsule creation
    b2Capsule capsule = {{center1.x*PIXELS_TO_METRES, center1.y*PIXELS_TO_METRES}, {center2.x*PIXELS_TO_METRES, center2.y*PIXELS_TO_METRES}, radius*PIXELS_TO_METRES};
    b2ShapeDef sd = b2DefaultShapeDef();
    b2CreateCapsuleShape(retId, &sd, &capsule);

    return retId;
}


/**
 * Finds the weighted center of a shape assuming that it has been partitioned into triangles
 * @param shapes a pointer to a list of traingle_polygon representing the traingles the shape has been partitioned into
 * @returns a vector2 representing the weighted center of the shape made up of these triangles
 */
vector2 ComputeWeightedCompoundCentroid(list *shapes) {
    vector2 weightedCentroid = {0.0f, 0.0f};
    float totalArea = 0.0f;

    for (int i = 0; i < shapes->size; i++) {
        //Getting the vertices
        vector2 a = get_value(shapes, triangle_polygon, i).points[0];
        vector2 b = get_value(shapes, triangle_polygon, i).points[1];
        vector2 c = get_value(shapes, triangle_polygon, i).points[2];

        //Getting the centroid of the triangle
        vector2 centroid = (vector2){(a.x + b.x + c.x) / 3.0f, (a.y + b.y + c.y) / 3.0f};

        //Getting the area
        float area = 0.5f * fabs(a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));

        //Accumulate weighted centroid sum:
        weightedCentroid.x += centroid.x * area;
        weightedCentroid.y += centroid.y * area;
        totalArea += area;
    }

    if (totalArea != 0) {
        weightedCentroid.x /= totalArea;
        weightedCentroid.y /= totalArea;
    }

    return weightedCentroid;
}

/**
 * Calculates the unweighted centroid of a shape by just finding the center of its bounding box
 * @param shapes a pointer to a list of traingle_polygon representing the traingles the shape has been partitioned into
 * @returns a vector2 representing the unweighted center of the shape made up of these triangles
 */
vector2 ComputeCompoundCentroid(list *shapes) {
    double minX = DBL_MAX, minY = DBL_MAX, maxX = -DBL_MAX, maxY = -DBL_MAX;

    //Get the bounding box:
    for(int i = 0; i < shapes->size; i++) {
        triangle_polygon tp = get_value(shapes, triangle_polygon, i);
        for (int j = 0; j < 3; j++) {
            if(tp.points[j].x < minX) minX = tp.points[j].x;
            if(tp.points[j].y < minY) minY = tp.points[j].y;
            if(tp.points[j].x > maxX) maxX = tp.points[j].x;
            if(tp.points[j].y > maxY) maxY = tp.points[j].y;
        }
    }
    return (vector2){minX + (maxX-minX)/2.0f, minY + (maxY-minY)/2.0f};
}

/**
 * Centers a shape paritioned into triangles around its centroid
 * @param shapes a pointer to a list of traingle_polygon representing the traingles the shape has been partitioned into
 * @param weighted whether the centroid should be weighted on unweighted
 */
void CenterCompundShape(list *shapes, bool weighted) {
    vector2 compoundCentroid = weighted ? ComputeWeightedCompoundCentroid(shapes) : ComputeCompoundCentroid(shapes);

    for (int i = 0; i < shapes->size; i++) {
        triangle_polygon tp = get_value(shapes, triangle_polygon, i);
        for (int j = 0; j < 3; j++) {
            tp.points[j].x -= compoundCentroid.x;
            tp.points[j].y -= compoundCentroid.y;
        }
        get_value(shapes, triangle_polygon, i) = tp;
    }
}

/**
 * POLYGON PARTITIONING
 * These functions are modified versions of those that can be found in the polypartition library: https://github.com/ivanfratric/polypartition
 * They have been modified to work in C rather than in C++.
 */

//Holds a state entry for the dynamic programming table
//Where i, j means this is the entry at index [j][i]
typedef struct {
    bool visible; //Whether diagonal (i, j) is valid
    double weight; //Minimum triangulation cost between (i, j)
    long bestvertex; //best splitting vertex k
} DPState;

/**
 * Holds two points on a diagonal
 */
typedef struct {
    long index1;
    long index2;
} Diagonal;

/**
 * Checks if the line formed by these three points is convex or not
 * @param p1 the first point in the line
 * @param p2 the second point in the line
 * @param p3 the third point in the line
 * @return whether the line is convex or not
 */
bool is_convex(vector2 *p1, vector2 *p2, vector2 *p3) {
    double tmp;
    // tmp = (p3->y - p1->y) * (p2->x - p1->x) - (p3->x - p1->x) * (p2->y - p1->y);
    tmp = (p2->x - p1->x)*(p3->y - p1->y) - (p2->y - p1->y)*(p3->x - p1->x);
    return tmp > 0;
}

/**
 * Checks if p lies in the cone formed by the lines p2 -> p1 and p2 -> p3
 * @param p1 the first point in the cone
 * @param p2 the middle point of the cone
 * @param p3 the third point in the cone
 * @param p the point being checked
 * @return true if p is in the cone, false otherwise
 */
bool in_cone(vector2 *p1, vector2 *p2, vector2 *p3, vector2 *p) {
  bool convex = is_convex(p1, p2, p3);

    if (convex) {
        if (!is_convex(p1, p2, p)) {
            return false;
        }
        if (!is_convex(p2, p3, p)) {
            return false;
        }
        return true;
    } else {
        if (is_convex(p1, p2, p)) {
            return true;
        }
        if (is_convex(p2, p3, p)) {
            return true;
        }
        return false;
    }
}

/**
 * Computes the signed area between three points
 * @param a a vector2 representing the first point
 * @param b a vector2 representing the second point
 * @param c a vector2 representing the third point
 * @return the signed area between these points
 */
double orient(vector2 *a, vector2 *b, vector2 *c) {
    return (b->x - a->x)*(c->y - a->y) -
           (b->y - a->y)*(c->x - a->x);
}

/**
 * Checks whether c lies on the line segment from a to b using AABB
 * @param a the first point of the line segment
 * @param b the second point of the line segment
 * @param c the point being tested
 * @return true if c lies on the line segment, false otherwise
 */
bool onSegment(vector2 *a, vector2 *b, vector2 *c) {
    return fmin(a->x,b->x) <= c->x && c->x <= fmax(a->x,b->x) &&
           fmin(a->y,b->y) <= c->y && c->y <= fmax(a->y,b->y);
}

/**
 * Determines whether two line segments intersect
 * @param p1 the start point of the first line segment
 * @param p2 the end point of the first line segment
 * @param p3 the start point of the second line segment
 * @param p4 the end point of the second line segment
 * @returns 1 if they intersect, 0 otherwise
 */
int intersects(vector2 *p1, vector2 *p2, vector2 *p3, vector2 *p4) {
    // ignore shared endpoints
    if ((p1->x == p3->x && p1->y == p3->y) ||
        (p1->x == p4->x && p1->y == p4->y) ||
        (p2->x == p3->x && p2->y == p3->y) ||
        (p2->x == p4->x && p2->y == p4->y))
        return 0;

    double o1 = orient(p1,p2,p3);
    double o2 = orient(p1,p2,p4);
    double o3 = orient(p3,p4,p1);
    double o4 = orient(p3,p4,p2);

    if (o1*o2 < 0 && o3*o4 < 0)
        return 1;

    // collinear cases
    if (o1 == 0 && onSegment(p1,p2,p3)) return 1;
    if (o2 == 0 && onSegment(p1,p2,p4)) return 1;
    if (o3 == 0 && onSegment(p3,p4,p1)) return 1;
    if (o4 == 0 && onSegment(p3,p4,p2)) return 1;

    return 0;
}

/**
 * Computes the optimal triangulation of a polygon without holes using dynamic programming.
 * Optimal in this case means the triangulation that minimises the total length of the added diagonals
 * @param poly the points of the polygon to triangulate. Must be in CCW order
 * @param n the number of points in the polygon
 * @param triangles a list where all the triangles are placed after traingulation
 * @return 1 on success, 0 on failure
 */
int triangulate_opt(vector2 *poly, int n, list *triangles) {
    long i, j, k, gap;
    DPState **dpstates = NULL; //Dynamic programming table
    vector2 p1, p2, p3, p4;
    long bestvertex;
    double weight, minweight, d1, d2;
    Diagonal diagonal, newdiagonal;
    queue *diagonals = queue_alloc(10, sizeof(Diagonal));
    triangle_polygon triangle;
    int ret = 1;

    dpstates = malloc(sizeof(DPState *)*n);
    for (i = 1; i < n; i++) {
        dpstates[i] = malloc(sizeof(DPState) * i);
    }

    // Initialize states and visibility before starting dynamic programming
    for (i = 0; i < (n - 1); i++) {
        p1 = poly[i];
        for (j = i + 1; j < n; j++) {
            dpstates[j][i].visible = true;
            dpstates[j][i].weight = 0;
            dpstates[j][i].bestvertex = -1;
            if (j != (i + 1)) {
                p2 = poly[j];

                // Visibility check. Checks that the diagonal lies inside the polygon angle
                if (i == 0) {
                    p3 = poly[n - 1];
                } else {
                    p3 = poly[i - 1];
                }
                if (i == (n - 1)) {
                    p4 = poly[0];
                } else {
                    p4 = poly[i + 1];
                }
                if (!in_cone(&p3, &p1, &p4, &p2)) {
                    // printf("Not incone\n");
                    dpstates[j][i].visible = false;
                    continue;
                }

                if (j == 0) {
                    p3 = poly[n - 1];
                } else {
                    p3 = poly[j - 1];
                }
                if (j == (n - 1)) {
                    p4 = poly[0];
                } else {
                    p4 = poly[j + 1];
                }
                if (!in_cone(&p3, &p2, &p4, &p1)) {
                    // printf("Not incone\n");
                    dpstates[j][i].visible = false;
                    continue;
                }

                for (k = 0; k < n; k++) {
                    p3 = poly[k];
                    if (k == (n - 1)) {
                        p4 = poly[0];
                    } else {
                        p4 = poly[k + 1];
                    }
                    //Checks that the diagonal does not intersect with a polygon edge
                    if (intersects(&p1, &p2, &p3, &p4)) {
                        // printf("Vertices intersect\n");
                        dpstates[j][i].visible = false;
                        break;
                    }
                }
            }
        }
    }
    // printf("n: %i\n", n);
    dpstates[n - 1][0].visible = true;
    dpstates[n - 1][0].weight = 0;
    dpstates[n - 1][0].bestvertex = -1;

    //Start dynamic programming
    for (gap = 2; gap < n; gap++) {
        for (i = 0; i < (n - gap); i++) {
            j = i + gap;
            if (!dpstates[j][i].visible) { //If the diagonal at this entry is not inside the polygon, continue
                // printf("DPStates at [j][i] is not visible\n");
                continue;
            }
            bestvertex = -1;
            minweight = DBL_MAX;
            //Try all possible splits for a diagonal
            for (k = (i + 1); k < j; k++) {
                //Ignore splits that are not in the polygon
                if (!dpstates[k][i].visible) {
                    // printf("DPStates at [k][i] is not visible\n");
                    continue;
                }
                if (!dpstates[j][k].visible) {
                    // printf("DPStates at [j][k] is not visible\n");
                    continue;
                }

                //Pick the k that minimises the total weight of all added diagonals
                if (k <= (i + 1)) {
                    d1 = 0;
                } else {
                    d1 = distance(&poly[i], &poly[k]);
                }
                if (j <= (k + 1)) {
                    d2 = 0;
                } else {
                    d2 = distance(&poly[k], &poly[j]);
                }

                weight = dpstates[k][i].weight + dpstates[j][k].weight + d1 + d2;

                if ((bestvertex == -1) || (weight < minweight)) {
                    // printf("Best vertex is -1 or weight < minweight\n");
                    bestvertex = k;
                    minweight = weight;
                }
            }
            if (bestvertex == -1) {
                for (i = 1; i < n; i++) {
                    free(dpstates[i]);
                }
                free(dpstates);

                return 0;
            }

            dpstates[j][i].bestvertex = bestvertex;
            dpstates[j][i].weight = minweight;
        }
    }

    //Reconstruct the triangulation after dynamic programming
    newdiagonal.index1 = 0;
    newdiagonal.index2 = n - 1;
    enqueue(diagonals, Diagonal, newdiagonal);
    bool ok;
    while (diagonals->size > 0) {
        dequeue(diagonals, Diagonal, diagonal, ok);
        if(!ok) break;
        bestvertex = dpstates[diagonal.index2][diagonal.index1].bestvertex;
        if (bestvertex == -1) {
            ret = 0;
            break;
        }
        triangle = (triangle_polygon){poly[diagonal.index1], poly[bestvertex], poly[diagonal.index2]};
        push_value(triangles, triangle_polygon, triangle);
        if (bestvertex > (diagonal.index1 + 1)) {
            newdiagonal.index1 = diagonal.index1;
            newdiagonal.index2 = bestvertex;
            enqueue(diagonals, Diagonal, newdiagonal);
        }
        if (diagonal.index2 > (bestvertex + 1)) {
            newdiagonal.index1 = bestvertex;
            newdiagonal.index2 = diagonal.index2;
            enqueue(diagonals, Diagonal, newdiagonal);
        }
    }

    for (i = 1; i < n; i++) {
        free(dpstates[i]);
    }
    free(dpstates);
    free_queue(diagonals);

    return ret;
}

/**
 * Removes adjacent colinear points as they can break the partitioning
 * @param poly the array of points to partition
 * @param n the size of the array of points
 * @return the new size of the array after removing all colinear points
 */
int remove_collinear(vector2 *poly, int n) {
    int write = 0;
    for (int i = 0; i < n; i++) {
        vector2 *prev = &poly[(i + n - 1) % n];
        vector2 *curr = &poly[i];
        vector2 *next = &poly[(i + 1) % n];
        double cross = (curr->x - prev->x) * (next->y - prev->y)
                     - (curr->y - prev->y) * (next->x - prev->x);
        if (cross != 0.0) {           // keep non-collinear vertices
            poly[write++] = *curr;
        }
    }
    return write;   // new n
}

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
b2BodyId create_polygon_collider(vector2* points, int points_size, vector2 center, float rotation, b2WorldId worldId, b2BodyType type) {
    printf("Center: (%f, %f)\n", center.x, center.y);
    //Converting the points to metre units from pixel ones
    vector2 *b2_points = malloc(sizeof(vector2) * points_size);
    for(int i = 0; i < points_size; i++) {
        b2_points[points_size - i - 1] = (vector2){(points[i].x) *PIXELS_TO_METRES, (points[i].y) *PIXELS_TO_METRES};
    }
    printf("Points to partition: \n");
    for(int i = 0; i < points_size; i++) {
        printf("(%f, %f) ", b2_points[i].x, b2_points[i].y);
    }
    printf("\n");
    //Default polygon initialisation
    b2BodyDef retBodyDef = b2DefaultBodyDef();
    retBodyDef.type = type;
    retBodyDef.position = (vector2){center.x * PIXELS_TO_METRES, center.y * PIXELS_TO_METRES};
    retBodyDef.rotation = (b2Rot){(float)cos(rotation), (float)sin(rotation)};
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);

    //I am going to partition the polygon regardless of whether or not the number of vertices is less than 8, because
    //Box2D does some very aggressive oversimplification of the shape outline which I'm not a fan of.
    //It is better to just put in triangles so it can't mess things up. I am going to use triangulation instead of
    //partitioning to make sure Box2D keeps all of the details, as in higher-vertex convex shapes there is a change
    //simplification could occur, which I want to avoid. This also helps to make sure that the vertices of the sub-polygons is
    //standardised.

    //Creating the list to hold the triangles generated by partitioning algorithm
    list *triangle_list = list_alloc(10, sizeof(triangle_polygon));

    points_size = remove_collinear(b2_points, points_size);
    if(points_size < 3) return retId;

    //Need to set it to be oriented Counter-Clockwise otherwise the triangulation algorithm fails.
    int result = triangulate_opt(b2_points, points_size, triangle_list); //Traingulate the polygon shape
    printf("Result: %i, ", result);
    printf("Number of triangles generated: %lu\n", triangle_list->size);
    printf("Triangle Vertices: \n");
    for(int i = 0; i < triangle_list->size; i++) {
        triangle_polygon t = get_value(triangle_list, triangle_polygon, i);
        for(int j = 0; j < 3; j++) {
            printf("(%f, %f) ", t.points[j].x, t.points[j].y);
        }
    }
    printf("\n");

    //Trying to center the polygon:
    CenterCompundShape(triangle_list, false);

    //Adding the polygons to the collider, or printing an error message if something goes wrong.
    for (int i = 0; i < triangle_list->size; i++) {
        vector2 *points = get_value(triangle_list, triangle_polygon, i).points;
        float area = 0.5f * fabs(
            points[0].x * (points[1].y - points[2].y) +
            points[1].x * (points[2].y - points[0].y) +
            points[2].x * (points[0].y - points[1].y)
        );

        if(area < 1e-6f) continue;
        b2Hull hull = b2ComputeHull(points, 3);
        if (hull.count == 0) {
            printf("Something odd has occured when generating a hull from a polyList\n");
        }
        else {
            b2Polygon testagon = b2MakePolygon(&hull, 0.0f);
            b2ShapeDef testshapeDef = b2DefaultShapeDef();
            b2ShapeId testShapeId = b2CreatePolygonShape(retId, &testshapeDef, &testagon);
            b2Shape_SetFriction(testShapeId, 0.3);
        }
    }
    printf("Center after creation: (%f, %f)\n", retBodyDef.position.x * METRES_TO_PIXELS, retBodyDef.position.y * METRES_TO_PIXELS);
    //Cleanup
    free_list(triangle_list);
    free(b2_points);
    return retId;
}

/**
 * Destroys an existing collider component
 * @param c a pointer to a collider component
 */
void free_collider(void *c) {
    collider *col = (collider *)c;
    b2DestroyBody(col->collider_id);
}

/**
 * Rotates a point by an angle
 * @param vector the point to rotate
 * @param angle the angle to rotate it by in radians
 * @return a new vector representing the rotated point
 */
vector2 rotate_translate(vector2* vector, float angle) {
    vector2 tmp;
    tmp.x = vector->x * cos(angle) - vector->y * sin(angle);
    tmp.y = vector->x * sin(angle) + vector->y * cos(angle);
    return tmp;
}

/**
 * Destroys an existing rigidbody component
 * @param rb a pointer to a rigidbody component
 */
void free_rigidbody(void *rb) {
    rigidbody *r = (rigidbody *)rb;
    free(r->pixel_coords);
}

button *create_button(void (*cb)(void *args), void *cb_args, ivector2 bounds) {
    button *ret = malloc(sizeof(button));
    ret->cb = cb;
    ret->cb_args = cb_args;
    ret->bounds = bounds;

    return ret;
}
void free_button(void *bt) {
    return;
}


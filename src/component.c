#include "../include/component.h"
#include "../include/list.h"
#include "../include/queue.h"
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

b2BodyId create_box_collider(vector2 center, int width, int height, float rotation, b2WorldId worldId, b2BodyType type) {
    //Default shape initialisation code
    b2BodyDef retBodyDef = b2DefaultBodyDef();
    retBodyDef.type = type;
    retBodyDef.position = (vector2){center.x * PIXELS_TO_METRES, center.y * PIXELS_TO_METRES};
    retBodyDef.rotation = (b2Rot){(float)cos(rotation), (float)sin(rotation)};
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);

    //Box creation
    b2Polygon box = b2MakeBox((width/2.0f)*PIXELS_TO_METRES, (height/2.0f)*PIXELS_TO_METRES);
    b2ShapeDef sd = b2DefaultShapeDef();
    b2CreatePolygonShape(retId, &sd, &box);

    return retId;
}

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

//Finds the center of a shape assuming that it has been partitioned into triangles
vector2 ComputeWeightedCompoundCentroid(list *shapes) {
    vector2 weightedCentroid = {0.0f, 0.0f};
    float totalArea = 0.0f;

    for (int i = 0; i < shapes->size; i++) {
        //Getting the vertices
        vector2 a = get_value(shapes, triangle_polygon, i).points[i];
        vector2 b = get_value(shapes, triangle_polygon, i).points[i];
        vector2 c = get_value(shapes, triangle_polygon, i).points[i];

        //Getting the centroid
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

//Calculates the unweighted centroid of a shape by just finding the center of its bounding box
vector2 ComputeCompoundCentroid(list *shapes) {
    //Get the bounding box:
    double minX = 0.0, minY = 0.0, maxX = 0.0, maxY = 0.0;

    for(int i = 0; i < shapes->size; i++) {
        triangle_polygon tp = get_value(shapes, triangle_polygon, i);
        for (int j = 0; j < 3; j++) {
            if(tp.points[j].x < minX) minX = tp.points[j].x;
            if(tp.points[j].y < minY) minY = tp.points[j].y;
            if(tp.points[j].x > maxX) maxX = tp.points[j].x;
            if(tp.points[j].y < maxY) maxX = tp.points[j].y;
        }
    }
    return (vector2){minX + (maxX-minX)/2.0f, minY + (maxY-minY)/2.0f};
}

//Centers a shape around its unweighted centroid
void CenterCompundShape(list *shapes) {
    vector2 compoundCentroid = ComputeCompoundCentroid(shapes);

    for (int i = 0; i < shapes->size; i++) {
        triangle_polygon tp = get_value(shapes, triangle_polygon, i);
        for (int j = 0; j < 3; j++) {
            tp.points[j].x -= compoundCentroid.x;
            tp.points[j].y -= compoundCentroid.y;
        }
    }
}

/**
 * These functions are modified versions of those that can be found in the polypartition library: https://github.com/ivanfratric/polypartition
 * They have been modified to work in C rather than in C++.
 */

typedef struct {
    bool visible;
    double weight;
    long bestvertex;
} DPState;

typedef struct {
    long index1;
    long index2;
} Diagonal;

bool IsConvex(vector2 *p1, vector2 *p2, vector2 *p3) {
    double tmp;
    tmp = (p3->y - p1->y) * (p2->x - p1->x) - (p3->x - p1->x) * (p2->y - p1->y);
    if (tmp > 0) {
        return 1;
    } else {
        return 0;
    }
}

bool InCone(vector2 *p1, vector2 *p2, vector2 *p3, vector2 *p) {
  bool convex;

  convex = IsConvex(p1, p2, p3);

    if (convex) {
        if (!IsConvex(p1, p2, p)) {
            return false;
        }
        if (!IsConvex(p2, p3, p)) {
            return false;
        }
        return true;
    } else {
        if (IsConvex(p1, p2, p)) {
            return true;
        }
        if (IsConvex(p2, p3, p)) {
            return true;
        }
        return false;
    }
}

int Intersects(vector2 *p11, vector2 *p12, vector2 *p21, vector2 *p22) {
    if ((p11->x == p21->x) && (p11->y == p21->y)) {
        return 0;
    }
    if ((p11->x == p22->x) && (p11->y == p22->y)) {
        return 0;
    }
    if ((p12->x == p21->x) && (p12->y == p21->y)) {
        return 0;
    }
    if ((p12->x == p22->x) && (p12->y == p22->y)) {
        return 0;
    }

    vector2 v1ort, v2ort, v;
    double dot11, dot12, dot21, dot22;

    v1ort.x = p12->y - p11->y;
    v1ort.y = p11->x - p12->x;

    v2ort.x = p22->y - p21->y;
    v2ort.y = p21->x - p22->x;

    v = (vector2){p21->x - p11->x, p21->y - p11->y};
    dot21 = v.x * v1ort.x + v.y * v1ort.y;
    v = (vector2){p22->x - p11->x, p22->y - p11->y};
    dot22 = v.x * v1ort.x + v.y * v1ort.y;

    v = (vector2){p11->x - p21->x, p11->y - p21->y};
    dot11 = v.x * v2ort.x + v.y * v2ort.y;
    v = (vector2){p12->x - p21->x, p12->y - p21->y};
    dot12 = v.x * v2ort.x + v.y * v2ort.y;

  if (dot11 * dot12 > 0) {
    return 0;
  }
  if (dot21 * dot22 > 0) {
    return 0;
  }

  return 1;
}

int triangulate_opt(vector2 *poly, int n, list *triangles) {
    // if (!poly->Valid()) {
    //   return 0;
    // }

    long i, j, k, gap;
    DPState **dpstates = NULL;
    vector2 p1, p2, p3, p4;
    long bestvertex;
    double weight, minweight, d1, d2;
    Diagonal diagonal, newdiagonal;
    queue *diagonals = queue_alloc(10, sizeof(Diagonal));
    triangle_polygon triangle;
    int ret = 1;

    dpstates = malloc(sizeof(DPState *)*n);
    for (i = 1; i < n; i++) {
        dpstates[i] = malloc(sizeof(DPState));
    }

    // Initialize states and visibility.
    for (i = 0; i < (n - 1); i++) {
        p1 = poly[i];
        for (j = i + 1; j < n; j++) {
            dpstates[j][i].visible = true;
            dpstates[j][i].weight = 0;
            dpstates[j][i].bestvertex = -1;
            if (j != (i + 1)) {
                p2 = poly[j];

                // Visibility check.
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
                if (!InCone(&p3, &p1, &p4, &p2)) {
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
                if (!InCone(&p3, &p2, &p4, &p1)) {
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
                    if (Intersects(&p1, &p2, &p3, &p4)) {
                        dpstates[j][i].visible = false;
                        break;
                    }
                }
            }
        }
    }
    dpstates[n - 1][0].visible = true;
    dpstates[n - 1][0].weight = 0;
    dpstates[n - 1][0].bestvertex = -1;

    for (gap = 2; gap < n; gap++) {
        for (i = 0; i < (n - gap); i++) {
            j = i + gap;
            if (!dpstates[j][i].visible) {
                continue;
            }
            bestvertex = -1;
            for (k = (i + 1); k < j; k++) {
                if (!dpstates[k][i].visible) {
                    continue;
                }
                if (!dpstates[j][k].visible) {
                    continue;
                }

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

    newdiagonal.index1 = 0;
    newdiagonal.index2 = n - 1;
    enqueue(diagonals, Diagonal, newdiagonal);
    while (!(diagonals->size > 0)) {
    // diagonal = *(diagonals.begin());
    // diagonals.pop_front();
    deqeue(diagonals, Diagonal, diagonal);
    bestvertex = dpstates[diagonal.index2][diagonal.index1].bestvertex;
    if (bestvertex == -1) {
        ret = 0;
        break;
    }
    // triangle.Triangle(poly->GetPoint(diagonal.index1), poly->GetPoint(bestvertex), poly->GetPoint(diagonal.index2));
    triangle = (triangle_polygon){poly[diagonal.index1], poly[bestvertex], poly[diagonal.index2]};
    // triangles->push_back(triangle);
    push_value(triangles, triangle_polygon, triangle);
    if (bestvertex > (diagonal.index1 + 1)) {
        newdiagonal.index1 = diagonal.index1;
        newdiagonal.index2 = bestvertex;
      // diagonals.push_back(newdiagonal);
        enqueue(diagonals, Diagonal, newdiagonal);
    }
    if (diagonal.index2 > (bestvertex + 1)) {
        newdiagonal.index1 = bestvertex;
        newdiagonal.index2 = diagonal.index2;
        // diagonals.push_back(newdiagonal);
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

b2BodyId create_polygon_collider(vector2* points, int pointsSize, vector2 center, float rotation, b2WorldId worldId, b2BodyType type) {
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

    //Creating the polygon for Polypartition
    // TPPLPoly* poly = new TPPLPoly();
    // poly->Init(pointsSize);
    // TPPLPolyList polyList;
    list *triangle_list = list_alloc(10, sizeof(triangle_polygon));

    // for (int i = 0; i < pointsSize; i++) {
    //     (*poly)[i].x = points[i].x;
    //     (*poly)[i].y = points[i].y;
    // }

    //Need to set it to be oriented Counter-Clockwise otherwise the triangulation algorithm fails.
    // poly->SetOrientation(TPPL_ORIENTATION_CCW); //This method does not actually check the order of each vertex. Need to change it so it sorts the points properly.
    // TPPLPartition test = TPPLPartition(); 
    int result = triangulate_opt(points, pointsSize, triangle_list); //Traingulate the polygon shape

    //Trying to center the polygon:
    CenterCompundShape(triangle_list);

    //Adding the polygons to the collider, or printing an error message if something goes wrong.
    for (int i = 0; i < triangle_list->size; i++) {
        b2Hull hull = b2ComputeHull(get_value(triangle_list, triangle_polygon, i).points, 3);
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
    free_list(triangle_list);
    return retId;
}

vector2 rotate_translate(vector2* vector, float angle) {
    vector2 tmp;
    tmp.x = vector->x * cos(angle) - vector->y * sin(angle);
    tmp.y = vector->x * sin(angle) + vector->y * cos(angle);
    return tmp;
}

void draw_collider(collider *c, debug_renderer *dRenderer, vector4 colour) {
    int shapeCount = b2Body_GetShapeCount(c->collider_id);
    vector2 colliderPosition = b2Body_GetPosition(c->collider_id);
    b2ShapeId* colliderShapes = malloc(sizeof(b2ShapeId) * shapeCount);
    b2Body_GetShapes(c->collider_id, colliderShapes, shapeCount);

    printf("New Collider:\n");
    //Need to draw the different collider types differently
    switch(c->type) {
        case BOX:
            for (int j = 0; j < shapeCount; j++) {
                vector2* colliderVertices = b2Shape_GetPolygon(colliderShapes[j]).vertices;
                vector2* rotatedVertices = (vector2*)malloc(4*sizeof(vector2));
                for (int k = 0; k < 4; k++) {
                    vector2 temp = rotate_translate(&colliderVertices[k], b2Rot_GetAngle(b2Body_GetRotation(c->collider_id)));
                    rotatedVertices[k] = (vector2){(temp.x + colliderPosition.x) * METRES_TO_PIXELS * PIXEL_SIZE, (temp.y + colliderPosition.y) * PIXEL_SIZE * METRES_TO_PIXELS};
                    printf("Vertex number %i: (%f, %f)\n", k, rotatedVertices[k].x, rotatedVertices[k].y);
                }
                render_draw_quad(dRenderer, rotatedVertices, colour);
                free(rotatedVertices);
            }
            break;
        case CIRCLE:
            for(int j = 0; j < shapeCount; j++) {
                b2Circle circle = b2Shape_GetCircle(colliderShapes[j]);
                render_draw_circle(dRenderer, (vector2){(circle.center.x+colliderPosition.x)* METRES_TO_PIXELS * PIXEL_SIZE, (circle.center.y+colliderPosition.y) * METRES_TO_PIXELS * PIXEL_SIZE}, circle.radius * METRES_TO_PIXELS * PIXEL_SIZE, colour);
            }
            break;
        case CAPSULE:
            for(int j = 0; j < shapeCount; j++) {
                b2Capsule capsule = b2Shape_GetCapsule(colliderShapes[j]);
                render_draw_circle(dRenderer, (vector2){(capsule.center1.x+colliderPosition.x)*METRES_TO_PIXELS*PIXEL_SIZE, (capsule.center1.y+colliderPosition.y) * METRES_TO_PIXELS * PIXEL_SIZE}, capsule.radius * METRES_TO_PIXELS * PIXEL_SIZE, colour);
                render_draw_circle(dRenderer, (vector2){(capsule.center2.x+colliderPosition.x)*METRES_TO_PIXELS*PIXEL_SIZE, (capsule.center2.y+colliderPosition.y) * METRES_TO_PIXELS * PIXEL_SIZE}, capsule.radius * METRES_TO_PIXELS * PIXEL_SIZE, colour);
            }
            break;
        case POLYGON:
            //Iterate over all of the subshapes, then just draw lines between the vertices
            for (int j = 0; j < shapeCount; j++) {
                vector2* colliderVertices = b2Shape_GetPolygon(colliderShapes[j]).vertices;
                vector2* rotatedVertices = (vector2*)malloc(3*sizeof(vector2));
                for (int k = 0; k < 3; k++) {
                    vector2 temp = rotate_translate(&colliderVertices[k], b2Rot_GetAngle(b2Body_GetRotation(c->collider_id)));
                    rotatedVertices[k] = (vector2){(temp.x + colliderPosition.x) * METRES_TO_PIXELS * PIXEL_SIZE, (temp.y + colliderPosition.y) * METRES_TO_PIXELS * PIXEL_SIZE};
                }
                for (int k = 0; k < 3; k++) {
                    render_draw_line(dRenderer, rotatedVertices[k], rotatedVertices[(k+1) % 3], colour);
                }
                free(rotatedVertices);
            }
            break;
        default:
            break;
    }
    free(colliderShapes);
}

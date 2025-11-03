#ifndef __RENDERER_H__
#define __RENDERER_H__
#include <stddef.h>
#include <stdint.h>
#include "../include/shader.h"
#include "../include/maths.h"
#include "../externals/cglm/cglm.h"

//Code adapted from this video: https://www.youtube.com/watch?v=NPnQF4yABwg

//Arbitrary constants for now
#define MAX_TRIANGLES 2048
#define MAX_QUADS 4096
#define VERTICES_PER_QUAD 4
#define VERTICES_PER_TRIANGLE 3
#define INDECIES_PER_QUAD 6
#define INDECIES_PER_TRIANGLE 3
#define MAX_VERTICES MAX_QUADS * VERTICES_PER_QUAD + MAX_TRIANGLES * VERTICES_PER_TRIANGLE
#define MAX_INDECIES MAX_QUADS * INDECIES_PER_QUAD + MAX_TRIANGLES * INDECIES_PER_TRIANGLE
#define MAX_TEXTURES 32
#define INVALID_TEX_INDEX 1248

#define MAX_POINTS 2048
#define MAX_DEBUG_VERTICES MAX_POINTS * 2
#define MAX_DEBUG_INDECIES MAX_POINTS * 16
#define CIRCLE_LINE_SEGEMENTS 64 //Number of line segments that make up the circumference of a circle

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

//Data structure to hold data about a single render vertex
typedef struct {
    vector2 pos;
    vector4 colour;
    vector2 uv;
    float tex_index;
} render_vertex;

typedef struct {
    float x;
    float y;
    float w;
    float h;
} quad;

typedef struct {
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
    shader shader;
    mat4 projection;

    render_vertex vertex_data[MAX_VERTICES];
    uint32_t index_data[MAX_INDECIES];

    uint32_t vertex_count;
    uint32_t index_count;

    uint32_t textures[MAX_TEXTURES]; //Textures added to the renderer in the current frame
    uint32_t texture_count; //Number of textures added to renderer in current frame. Always <= MAX_TEXTURES
} renderer;

//Allocate the renderer and assign its variables
void render_init(renderer *r, char *vertPath, char *fragPath);
//Destroy the renderer
void render_free(renderer *r);
//Begin a single render frame (this is equivalent to a gpu render call)
void render_begin_frame(renderer *r);
//End a single render frame
void render_end_frame(renderer *r);
//Add a triangle polygon to the current render frame
void render_push_triangle(renderer *r, vector2 coords[3], vector4 colours[3], vector2 uv[3], uint32_t texture);
//A a quad polygon to the current frame
void render_push_quad(renderer *r, vector2 coords[4], vector4 colours[4], vector2 uv[4], uint32_t texture);
//Load a texture
uint32_t render_texture_load(char *filepath);

typedef struct {
    vector2 position;
    vector4 colour;
} debug_render_vertex;

typedef struct {
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
    shader shader;
    mat4 projection;

    size_t point_count;
    size_t vertex_count;
    size_t index_count;

    debug_render_vertex points[MAX_POINTS];
    debug_render_vertex vertices[MAX_DEBUG_VERTICES];
    uint32_t index_data[MAX_DEBUG_INDECIES];
} debug_renderer;

//Allocate the renderer and assign its variables
void debug_render_init(debug_renderer *r, char *vertPath, char *fragPath);
//Destroy the renderer
void debug_render_free(debug_renderer *r);
//Begin a single render frame (this is equivalent to a gpu render call)
void debug_render_flush(debug_renderer *r);
//Renders a quad on the screen
void render_draw_quad(debug_renderer *r, quad *dimensions, vector4 colour);
//Draws a line between two points
void render_draw_line(debug_renderer*r, vector2 start, vector2 end, vector4 colour);
//Draws a point
void render_draw_point(debug_renderer *r, vector2 position, vector4 colour);
//Draws a circle
void render_draw_circle(debug_renderer* r, vector2 center, float radius, vector4 colour);

#endif

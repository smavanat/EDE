#ifndef __RENDERER_H__
#define __RENDERER_H__
#include <stdint.h>
#include "../include/shader.h"
#include "../include/maths.h"

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
// void render_push_triangle(renderer *r, vector2 a, vector2 b, vector2 c, vector4 a_colour, vector4 b_colour, vector4 c_colour, vector2 a_uv, vector2 b_uv, vector2 c_uv, uint32_t texture);

#endif

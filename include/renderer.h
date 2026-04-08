#ifndef __RENDERER_H__
#define __RENDERER_H__
#include <stddef.h>
#include <stdint.h>
#include "../include/component.h"
#include "../include/shader.h"
#include "../include/maths.h"
#include "../externals/cglm/cglm.h"

//Code adapted from this video: https://www.youtube.com/watch?v=NPnQF4yABwg

//Arbitrary constants for now
#define MAX_TRIANGLES 2048
#define MAX_QUADS 4096
#define MAX_TEXTURES 32

#define VERTICES_PER_QUAD 4
#define VERTICES_PER_TRIANGLE 3
#define INDECIES_PER_QUAD 6
#define INDECIES_PER_TRIANGLE 3
#define MAX_VERTICES MAX_QUADS * VERTICES_PER_QUAD + MAX_TRIANGLES * VERTICES_PER_TRIANGLE
#define MAX_INDECIES MAX_QUADS * INDECIES_PER_QUAD + MAX_TRIANGLES * INDECIES_PER_TRIANGLE
#define INVALID_TEX_INDEX 1248

#define MAX_POINTS 2048
#define MAX_DEBUG_VERTICES MAX_POINTS * 2
#define MAX_DEBUG_INDECIES MAX_POINTS * 16
#define CIRCLE_LINE_SEGEMENTS 64 //Number of line segments that make up the circumference of a circle

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define PIXEL_SIZE 10
#define PIXEL_SCREEN_WIDTH SCREEN_WIDTH / PIXEL_SIZE
#define PIXEL_SCREEN_HEIGHT SCREEN_HEIGHT / PIXEL_SIZE

//Data structure to hold data about a single render vertex
typedef struct {
    vector2 pos; //The on-screen position of the render vertex
    vector4 colour; //The colour of the vertex
    vector2 uv; //The (u,v) coordinates of the vertex
    float tex_index; //The index of the texture that this vertex is rendering from
} render_vertex;

typedef struct {
    float x;
    float y;
    float w;
    float h;
} quad;

//Renderer for rendering normal textures
typedef struct {
    uint32_t vao; //vao this renderer uses
    uint32_t vbo; //vbo this renderer uses
    uint32_t ebo; //ebo this renderer uses
    shader shader; //shader this renderer uses
    mat4 projection; //projection matrix for this renderer

    render_vertex vertex_data[MAX_VERTICES]; //The render vertex data for this renderer
    uint32_t index_data[MAX_INDECIES]; //The index count (for ebo) for this renderer

    uint32_t vertex_count; //Number of vertices
    uint32_t index_count; //Number of indecies

    uint32_t textures[MAX_TEXTURES]; //Textures added to the renderer in the current frame
    uint32_t texture_count; //Number of textures added to renderer in current frame. Always <= MAX_TEXTURES
} renderer;

/**
 * Allocate the renderer and assign its variables
 * @param r a pointer to the renderer to allocate
 * @param vert_path the path to load the vertex shader for the renderer from
 * @param frag_path the path to load the fragment shader for the renderer from
 */
void render_init(renderer *r, char *vertPath, char *fragPath);
/**
 * Freeing the renderer
 * @param r a pointer to the renderer to free
 */
void render_free(renderer *r);
/**
 * Resetting the renderer for a new draw call
 * @param r the renderer to reset
 */
void render_begin_frame(renderer *r);
/**
 * Renderering the current stored data at the end of a draw call
 * @param r the renderer to render from
 */
void render_end_frame(renderer *r);
/**
 * Add a triangle polygon to the current render frame
 * @param r the renderer to use
 * @param coords the coordinates of the triangle to render
 * @param colours the colours of each of the triangle vertices
 * @param uv the (u,v) coordinates of the triangle
 * @param texture the texture to render from
 */
void render_push_triangle(renderer *r, vector2 coords[3], vector4 colours[3], vector2 uv[3], uint32_t texture);
/**
 * A a quad polygon to the current frame
 * @param r the renderer to use
 * @param coords the coordinates of the quad to render
 * @param colours the colours of each of the quad vertices
 * @param uv the (u,v) coordinates of the quad
 * @param texture the texture to render from
 */
void render_push_quad(renderer *r, vector2 coords[4], vector4 colours[4], vector2 uv[4], uint32_t texture);
/**
 * Load a texture
 * @param filepath the filepath to load the texture from
 * @return the id of the OpenGL texture object created to represent the texture
 */
uint32_t render_texture_load(char *filepath);

//A renderer used for rendering pixels like in Noita
typedef struct {
    uint32_t vao; //vao this renderer uses
    uint32_t vbo; //vbo this renderer uses
    uint32_t ebo; //ebo this renderer uses
    shader shader; //shader this renderer uses
    mat4 projection; //projection matrix for this renderer

    //Stuff for pixel rendering:
    uint32_t pbo; //pbo this renderer uses
    uint8_t *pixels; //Array of pixel data
    uint32_t pixel_tex; //The texture the pbo is rendered to
} pixel_renderer;

/**
 * Initialises the pixel renderer
 * @param r the pixel renderer to initialise
 * @param vert_path the filepath to load the vertex shader from
 * @param frag_path the filepath to load the fragment shader from
 */
void pixel_render_init(pixel_renderer *r, char *vertPath, char *fragPath);
/**
 * Frees a pixel renderer
 * @param r the pixel renderer to free
 */
void pixel_render_free(pixel_renderer *r);
/**
 * Sets up the variables for renderering to the pbo from the pixel_renderer
 * @param r the pixel render to begin the pixel frame for
 */
void render_begin_pixel_frame(pixel_renderer *r);
/**
 * Ends rendering to the current pixel frame
 * @param r the pixel_renderer whose frame should end
 */
void render_end_pixel_frame(pixel_renderer *r);
/**
 * Draws a pixel to the pixel buffer
 * @param r the pixel renderer to render to
 * @param position where the pixel should be rendererd in the texture
 * @param colour the colour of the pixel
 */
void draw_pixel(pixel_renderer *r, uint32_t position, uint8_t colour[4]);
/**
 * Draws the entire grid directly on the screen by copying its entire contents into the renderer's pixel buffer
 * @param r the pixel renderer to render to
 * @oaram g the pixel grid whose pixels to use
 */
void draw_grid(pixel_renderer *r, world_grid *g);

//Vertex for debugging. Simpler than the other version since its rendered directly onto the screen and not from a texture
typedef struct {
    vector2 position; //On-screen position of the vertex
    vector4 colour; //rgba colour of the vertex
} debug_render_vertex;

//Used to render pimitives (quads, lines, points and circles) for debugging purposes
typedef struct {
    uint32_t vao; //vao this renderer uses
    uint32_t vbo; //vbo this renderer uses
    uint32_t ebo; //ebo this renderer uses
    shader shader; //shader this renderer uses
    mat4 projection; //projection matrix for this renderer

    size_t point_count; //Separate count for points since can draw them differenttly to other primites (since all other primitives are just lines)
    size_t vertex_count; //Number of line vertices
    size_t index_count; //Number of ebo indecies

    debug_render_vertex points[MAX_POINTS]; //Array for holding the points
    debug_render_vertex vertices[MAX_DEBUG_VERTICES]; //Array for holding the vertices
    uint32_t index_data[MAX_DEBUG_INDECIES]; //Array for holding the indecies
} debug_renderer;

/**
 * Allocate the renderer and assign its variables
 * @param r the debug renderer to initialise
 * @param vert_path the filepath to load the vertex shader from
 * @param frag_path the filepath to load the fragment shader from
 */
void debug_render_init(debug_renderer *r, char *vertPath, char *fragPath);
/*
 * Free the renderer
 * @param r the renderer to free
 */
void debug_render_free(debug_renderer *r);
/*
 * Begin a single render frame (this is equivalent to a gpu render call)
 * @param r the debug_renderer whose render frame needs to start
 */
void debug_render_flush(debug_renderer *r);
/*
 * Renders an unfilled quad on the screen. The quad can be irregular. Do not use triangles, since this adds a diagonal in the middle, just use four lines
 * @param r the debug_renderer to use
 * @param dimensions the coordinates of the corners of the quad
 * @param colour the colour of the outline of the quad
 */
void render_draw_quad(debug_renderer *r, vector2 *dimensions, vector4 colour);
/*
 * Draws a line between two points
 * @param r the debug_renderer to use
 * @param start the start of the line
 * @param end the end of the line
 * @param colour the colour of the line
 */
void render_draw_line(debug_renderer*r, vector2 start, vector2 end, vector4 colour);
/*
 * Draws a point
 * @param r the debug_renderer to use
 * @param position where to drawn the point
 * @param colour the colour of the point
 */
void render_draw_point(debug_renderer *r, vector2 position, vector4 colour);
/*
 * Draws an unfilled circle. A circle is just lots of small lines (number of which is defined by CIRCLE_LINE_SEGMENTS) angled a small amount
 * @param r the debug_renderer to use
 * @param center the center of the circle
 * @param radius the radius of the circle
 * @param colour the colour of the circle
 */
void render_draw_circle(debug_renderer* r, vector2 center, float radius, vector4 colour);
/**
 * Draws collider outlines for debugging purposes
 * @param c a pointer to the collider whose outline needs to be drawn
 * @param dRenderer a pointer to the debug_renderer doing the drawing
 * @param colour the colour of the outline
 */
void draw_collider(collider *c, debug_renderer *dRenderer, vector4 colour);

#endif

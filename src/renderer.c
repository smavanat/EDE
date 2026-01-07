#include "../include/renderer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include"../include/stb_image.h"

//Allocate the renderer and assign its variables
void render_init(renderer *r, char *vertPath, char *fragPath) {
    //Getting the vao
    glGenVertexArrays(1, &r->vao);
    glBindVertexArray(r->vao);

    //Getting the vbo
    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(render_vertex), NULL, GL_DYNAMIC_DRAW);

    //Getting the ebo
    glGenBuffers(1, &r->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * MAX_INDECIES, NULL, GL_DYNAMIC_DRAW);

    //Setting the vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)offsetof(render_vertex, pos)); //Vertex Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)offsetof(render_vertex, colour)); //Vertex Colour
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)offsetof(render_vertex, uv)); //UV
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)offsetof(render_vertex, tex_index)); //Texture Index
    glEnableVertexAttribArray(3);

    //Getting the shader for this renderer
    r->shader = load_shader(vertPath, fragPath);

    //Setting the projection matrix
    glm_ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, -1.0f, 1.0f, r->projection);

    uint32_t tex_loc = glGetUniformLocation(r->shader, "u_tex");
    int32_t textures[MAX_TEXTURES] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    glUniform1iv(tex_loc, MAX_TEXTURES, textures);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//Destroying the renderer
void render_free(renderer *r) {
    glDeleteBuffers(1, &r->vbo);
    glDeleteVertexArrays(1, &r->vao);

    glDeleteProgram(r->shader);

    for (int i = 0; i < r->texture_count; i++) {
        glDeleteTextures(1, &r->textures[i]);
    }
}

//Resetting the renderer for a new draw call
void render_begin_frame(renderer *r) {
    r->vertex_count = 0;
    r->index_count = 0;
    r->texture_count = 0;
}

//Renderering the current stored data at the end of a draw call
void render_end_frame(renderer *r) {
    //Shifting the positions according to the projection matrix
    use(r->shader);
    int proj_loc = glGetUniformLocation(r->shader, "uProjection");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (float *)r->projection);

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->vertex_count * sizeof(render_vertex), r->vertex_data); //Copies the data from renderer's triangle data into the vbo

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, r->index_count * sizeof(uint32_t), r->index_data); //Copies the quad data into the vbo

    for(int i = 0; i < r->texture_count; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, r->textures[i]);
    }

    glUseProgram(r->shader);
    glBindVertexArray(r->vao);

    glDrawElements(GL_TRIANGLES, r->index_count, GL_UNSIGNED_INT, 0);
}

//Add a triangle polygon to the current render frame
void render_push_triangle(renderer *r, vector2 coords[3], vector4 colours[3], vector2 uv[3], uint32_t texture) {
    uint32_t tex_index = INVALID_TEX_INDEX; //Setting default value to invalid so successful operations can make the index valid again

    //Check to see if the texture already exists in the renderer's current draw call
    for(int i = 0; i < r->texture_count; i++) {
        if(r->textures[i] == texture){
            tex_index = i;
            break;
        }
    }

    //If the texture is not currently held
    //r->texture_count < MAX_TEXTURES confirms we don't write more than the available texture slots
    if(tex_index == INVALID_TEX_INDEX && r->texture_count < MAX_TEXTURES) {
        r->textures[r->texture_count] = texture;
        tex_index = r->texture_count;
        r->texture_count++;
    }

    //If we have overreached our current rendering limit or we cannot store any more textures, end the current draw call and start a new one
    if(r->vertex_count + VERTICES_PER_TRIANGLE >= MAX_VERTICES || r->index_count + INDECIES_PER_TRIANGLE >= MAX_INDECIES || tex_index == INVALID_TEX_INDEX) {
        render_end_frame(r);
        render_begin_frame(r);
    }

    //Update the vertex count and vertex data stored in the renderer
    uint32_t base_index = r->vertex_count;

    for(int i = 0; i < VERTICES_PER_TRIANGLE; i++) {
        r->vertex_data[r->vertex_count++] = (render_vertex){coords[i], colours[i], uv[i], tex_index};
        r->index_data[r->index_count++] = base_index + i;
    }
}

//A a quad polygon to the current frame
void render_push_quad(renderer *r, vector2 coords[4], vector4 colours[4], vector2 uv[4], uint32_t texture) {
    uint32_t tex_index = INVALID_TEX_INDEX; //Setting default value to invalid so successful operations can make the index valid again

    //Check to see if the texture already exists in the renderer's current draw call
    for(int i = 0; i < r->texture_count; i++) {
        if(r->textures[i] == texture){
            tex_index = i;
            break;
        }
    }

    //If the texture is not currently held
    //r->texture_count < MAX_TEXTURES confirms we don't write more than the available texture slots
    if(tex_index == INVALID_TEX_INDEX && r->texture_count < MAX_TEXTURES) {
        r->textures[r->texture_count] = texture;
        tex_index = r->texture_count;
        r->texture_count++;
    }

    //If we have overreached our current rendering limit or we cannot store any more textures, end the current draw call and start a new one
    if(r->vertex_count + VERTICES_PER_QUAD >= MAX_VERTICES || r->index_count + INDECIES_PER_QUAD >= MAX_INDECIES || tex_index == INVALID_TEX_INDEX) {
        render_end_frame(r);
        render_begin_frame(r);
    }

    //Update the vertex count and vertex data stored in the renderer
    uint32_t base_index = r->vertex_count;

    for(int i = 0; i < VERTICES_PER_QUAD; i++) {
        r->vertex_data[r->vertex_count++] = (render_vertex){coords[i], colours[i], uv[i], tex_index};
    }

    //Need to also add ebo data so we can remove overlapping vertices
    //First triangle
    r->index_data[r->index_count++] = base_index;
    r->index_data[r->index_count++] = base_index + 1;
    r->index_data[r->index_count++] = base_index + 3;

    //Second triangle
    r->index_data[r->index_count++] = base_index + 1;
    r->index_data[r->index_count++] = base_index + 2;
    r->index_data[r->index_count++] = base_index + 3;
}

//Load a texture
uint32_t render_texture_load(char *filepath) {
    uint32_t id;
    // load and create a texture
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    unsigned char *data = stbi_load("../data/assets/container.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture\n");
        return 0;
    }
    stbi_image_free(data);
    return id;
}

/**
 * CODE FOR THE PIXEL RENDERER
 */

void pixel_render_init(pixel_renderer *r, char *vertPath, char *fragPath) {
    //Setting up the pbo for the pixel simulations:
    glGenTextures(1, &r->pixel_tex);
    glBindTexture(GL_TEXTURE_2D, r->pixel_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, PIXEL_SCREEN_WIDTH, PIXEL_SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenBuffers(1, &r->pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, r->pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, PIXEL_SCREEN_HEIGHT * PIXEL_SCREEN_WIDTH * 4, NULL, GL_STREAM_DRAW);
    r->pixels = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if(r->pixels) {
        memset(r->pixels, 0x00, PIXEL_SCREEN_HEIGHT * PIXEL_SCREEN_WIDTH * 4);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    r->shader = load_shader(vertPath, fragPath);

    float quadVertices[] = {
        // positions                                // texCoords
        0.0f, 0.0f,  0.0f, 0.0f,                            // top-left
        0.0f, SCREEN_HEIGHT,  0.0f, 1.0f,             // bottom-left
        SCREEN_WIDTH, SCREEN_HEIGHT,  1.0f, 1.0f, // bottom-right
        SCREEN_WIDTH, 0.0f,  1.0f, 0.0f               // top-right
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &r->vao);
    glGenBuffers(1, &r->vbo);
    glGenBuffers(1, &r->ebo);

    glBindVertexArray(r->vao);

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Setting the projection matrix
    glm_ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, -1.0f, 1.0f, r->projection);
}

void pixel_render_free(pixel_renderer *r) {
    glDeleteBuffers(1, &r->vbo);
    glDeleteBuffers(1, &r->pbo);
    glDeleteVertexArrays(1, &r->vao);

    glDeleteProgram(r->shader);
    glDeleteTextures(1, &r->pixel_tex);
}

void render_begin_pixel_frame(pixel_renderer *r) {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, r->pbo); //Binding the pbo
    glBufferData(GL_PIXEL_UNPACK_BUFFER, PIXEL_SCREEN_HEIGHT * PIXEL_SCREEN_WIDTH * 4, NULL, GL_STREAM_DRAW); //Forcing the gpu to discard any data it is currently using
    r->pixels = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if(r->pixels) {
        memset(r->pixels, 0x00, PIXEL_SCREEN_HEIGHT * PIXEL_SCREEN_WIDTH * 4);
    }
}

void render_end_pixel_frame(pixel_renderer *r) {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, r->pbo);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

    glBindTexture(GL_TEXTURE_2D, r->pixel_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0,0,0, PIXEL_SCREEN_WIDTH, PIXEL_SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    use(r->shader);
    int proj_loc = glGetUniformLocation(r->shader, "uProjection");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (float *)r->projection);

    // Bind texture to correct unit and uniform
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->pixel_tex);
    glUniform1i(glGetUniformLocation(r->shader, "screenTexture"), 0);
    glBindVertexArray(r->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void draw_pixel(pixel_renderer *r, uint32_t position, uint8_t colour[4]) {
    if(!r->pixels) {
        printf("Pixel Buffer is NULL!!\n");
        return;
    }
    if (position >= 0 && position < PIXEL_SCREEN_HEIGHT * PIXEL_SCREEN_WIDTH) {
        for(int i = 0; i < 4; i++) {
            r->pixels[(position*4)+i] = colour[i];
        }
    }
}

/*
 * CODE FOR THE DEBUG RENDERER
 */

//Allocate the renderer and assign its variables
void debug_render_init(debug_renderer *r, char *vertPath, char *fragPath) {
    //Getting the vao
    glGenVertexArrays(1, &r->vao);
    glBindVertexArray(r->vao);

    //Getting the vbo
    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(debug_render_vertex), NULL, GL_DYNAMIC_DRAW);

    //Getting the ebo
    glGenBuffers(1, &r->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * MAX_INDECIES, NULL, GL_DYNAMIC_DRAW);

    //Setting the vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(debug_render_vertex), (void *)offsetof(debug_render_vertex, position)); //Vertex Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(debug_render_vertex), (void *)offsetof(debug_render_vertex, colour)); //Vertex Colour
    glEnableVertexAttribArray(1);

    //Getting the shader for this renderer
    r->shader = load_shader(vertPath, fragPath);

    //Setting the projection matrix
    glm_ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, -1.0f, 1.0f, r->projection);

    r->point_count = 0;
    r->vertex_count = 0;
    r->index_count = 0;

    glPointSize(10.0f);
}

//Destroy the renderer
void debug_render_free(debug_renderer *r) {
    glDeleteBuffers(1, &r->vbo);
    glDeleteVertexArrays(1, &r->vao);

    glDeleteProgram(r->shader);
}

//Begin a single render frame (this is equivalent to a gpu render call)
void debug_render_flush(debug_renderer *r) {
    use(r->shader);

    //Shifting the positions according to the projection matrix
    int proj_loc = glGetUniformLocation(r->shader, "uProjection");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (float *)r->projection);

    glBindVertexArray(r->vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);

    //Drawing points:
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->point_count * sizeof(debug_render_vertex), r->points);
    glDrawArrays(GL_POINTS, 0, r->point_count);

    //Drawing everything else:
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->vertex_count * sizeof(debug_render_vertex), r->vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, r->index_count * sizeof(uint32_t), r->index_data);
    glDrawElements(GL_LINES, r->index_count, GL_UNSIGNED_INT, 0);


    //Need to reset the counts to flush the data out
    r->point_count = 0;
    r->vertex_count = 0;
    r->index_count = 0;
}

//NOTE: For now, there is just a hard limit on the number of objects that can be renderered by the debug renderer perframe. Will change this as necessary

//Renders a quad on the screen
void render_draw_quad(debug_renderer *r, vector2 *dimensions, vector4 colour) {
    if(r->vertex_count+ 4 >= MAX_DEBUG_VERTICES) {
        printf("Max amount of quads reached for this frame");
        return;
    }
    uint32_t base_index = r->vertex_count;

    for(int i = 0; i < 4; i++) {
        r->vertices[r->vertex_count++] = (debug_render_vertex){{dimensions[i].x, dimensions[i].y}, colour};
    }

    // r->vertices[r->vertex_count++] = (debug_render_vertex){{dimensions->x, dimensions->y}, colour}; // top-left
    // r->vertices[r->vertex_count++] = (debug_render_vertex){{dimensions->x + dimensions->w, dimensions->y}, colour}; // top-right
    // r->vertices[r->vertex_count++] = (debug_render_vertex){{dimensions->x + dimensions->w, dimensions->y + dimensions->h}, colour}; // bottom-right
    // r->vertices[r->vertex_count++] = (debug_render_vertex){{dimensions->x, dimensions->y + dimensions->h}, colour}; // bottom-left

    //Need to also add ebo data so we can remove overlapping vertices
    //Unfortunately have to draw debug quads as 4 lines otherwise we get an ugly diagonal line in the middle because they're actually two quads
    // Assume `base_index` is the first vertex of this quad in r->quads
    r->index_data[r->index_count++] = base_index + 0; r->index_data[r->index_count++] = base_index + 1; // top edge
    r->index_data[r->index_count++] = base_index + 1; r->index_data[r->index_count++] = base_index + 2; // right edge
    r->index_data[r->index_count++] = base_index + 2; r->index_data[r->index_count++] = base_index + 3; // bottom edge
    r->index_data[r->index_count++] = base_index + 3; r->index_data[r->index_count++] = base_index + 0; // left edge
}

//Draws a line between two points
void render_draw_line(debug_renderer*r, vector2 start, vector2 end, vector4 colour) {
    if(r->vertex_count + 2 >= MAX_DEBUG_VERTICES) {
        printf("Max amount of lines reached for this frame");
        return;
    }

    uint32_t base = r->vertex_count;
    r->vertices[r->vertex_count++] = (debug_render_vertex){start, colour};
    r->vertices[r->vertex_count++] = (debug_render_vertex){end, colour};

    r->index_data[r->index_count++] = base;
    r->index_data[r->index_count++] = base+1;
}

//Draws a point
void render_draw_point(debug_renderer *r, vector2 position, vector4 colour) {
    if(r->point_count + 1 >= MAX_POINTS) {
        printf("Max amount of vertices reached for this frame");
        return;
    }
    r->points[r->point_count++] = (debug_render_vertex){position, colour};
}

//Draws a circle
void render_draw_circle(debug_renderer* r, vector2 center, float radius, vector4 colour) {
    if(r->vertex_count + CIRCLE_LINE_SEGEMENTS >= MAX_DEBUG_VERTICES) {
        printf("Max amount of circles reached for this frame");
        return;
    }

    float angle_step = 2.0f * M_PI / CIRCLE_LINE_SEGEMENTS;
    float aspect = 800.0f/ 600.0f; //Temporary. Need to write a projection matrix to fix weird stretching bugs

    uint32_t base_index = r->vertex_count;
    for(int i = 0; i < CIRCLE_LINE_SEGEMENTS; i++) {
        float angle = i * angle_step;
        float x = center.x + cosf(angle) * radius ;//* (1.0f /aspect);
        float y = center.y + sinf(angle) * radius;
        r->vertices[r->vertex_count++] = (debug_render_vertex){(vector2){x, y}, colour};
    }

    for(int i = 0; i < CIRCLE_LINE_SEGEMENTS; i++) {
        r->index_data[r->index_count + (i*2)] = base_index + i;
        r->index_data[r->index_count + (i*2) + 1] = base_index + ((i+1) % CIRCLE_LINE_SEGEMENTS);
    }

    r->index_count += CIRCLE_LINE_SEGEMENTS * 2;
}

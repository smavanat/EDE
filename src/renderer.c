#include "../include/renderer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

void render_init(renderer *r, char *vertPath, char *fragPath) {
    glGenVertexArrays(1, &r->vao);
    glBindVertexArray(r->vao);

    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(render_vertex), NULL, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &r->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * MAX_INDECIES, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)offsetof(render_vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)offsetof(render_vertex, colour));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)offsetof(render_vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)offsetof(render_vertex, tex_index));
    glEnableVertexAttribArray(0);

    r->shader = load_shader(vertPath, fragPath);
    use(r->shader);

    uint32_t tex_loc = glGetUniformLocation(r->shader, "u_tex");
    int32_t textures[MAX_TEXTURES] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    glUniform1iv(tex_loc, MAX_TEXTURES, textures);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void render_free(renderer *r) {
    glDeleteBuffers(1, &r->vbo);
    glDeleteVertexArrays(1, &r->vao);

    glDeleteProgram(r->shader);

    for (int i = 0; i < r->texture_count; i++) {
        glDeleteTextures(1, &r->textures[i]);
    }
}

void render_begin_frame(renderer *r) {
    glClear(GL_COLOR_BUFFER_BIT);

    r->vertex_count = 0;
    r->index_count = 0;
    r->texture_count = 0;
}

void render_end_frame(renderer *r) {
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

void render_push_triangle(renderer *r, vector2 coords[3], vector4 colours[3], vector2 uv[3], uint32_t texture) {
    uint32_t tex_index = INVALID_TEX_INDEX;
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

    if(r->vertex_count + VERTICES_PER_TRIANGLE >= MAX_VERTICES || r->index_count + INDECIES_PER_TRIANGLE >= MAX_INDECIES || tex_index == INVALID_TEX_INDEX) {
        render_end_frame(r);
        render_begin_frame(r);
    }

    uint32_t base_index = r->vertex_count;

    for(int i = 0; i < VERTICES_PER_TRIANGLE; i++) {
        r->vertex_data[r->vertex_count++] = (render_vertex){coords[i], colours[i], uv[i], tex_index};
        r->index_data[r->index_count++] = base_index + i;
    }
}

// void render_push_triangle(renderer *r, vector2 a, vector2 b, vector2 c, vector4 a_colour, vector4 b_colour, vector4 c_colour, vector2 a_uv, vector2 b_uv, vector2 c_uv, uint32_t texture) {
//     // 1248 is just an invalid value since this is an unsigned number so -1 doesn't work
//     uint32_t tex_index = INVALID_TEX_INDEX;
//     for(int i = 0; i < r->texture_count; i++) {
//         if(r->textures[i] == texture){
//             tex_index = i;
//             break;
//         }
//     }
//
//     //If the texture is not currently held
//     //r->texture_count < MAX_TEXTURES confirms we don't write more than the available texture slots
//     if(tex_index == INVALID_TEX_INDEX && r->texture_count < MAX_TEXTURES) {
//         r->textures[r->texture_count] = texture;
//         tex_index = r->texture_count;
//         r->texture_count++;
//     }
//
//     r->triangle_data[r->triangle_count * 3 + 0].pos = a;
//     r->triangle_data[r->triangle_count * 3 + 0].colour = a_colour;
//     r->triangle_data[r->triangle_count * 3 + 0].uv= a_uv;
//     r->triangle_data[r->triangle_count * 3 + 0].tex_index= tex_index;
//     r->triangle_data[r->triangle_count * 3 + 1].pos = b;
//     r->triangle_data[r->triangle_count * 3 + 1].colour = b_colour;
//     r->triangle_data[r->triangle_count * 3 + 1].uv= b_uv;
//     r->triangle_data[r->triangle_count * 3 + 1].tex_index= tex_index;
//     r->triangle_data[r->triangle_count * 3 + 2].pos = c;
//     r->triangle_data[r->triangle_count * 3 + 2].colour = c_colour;
//     r->triangle_data[r->triangle_count * 3 + 2].uv= c_uv;
//     r->triangle_data[r->triangle_count * 3 + 2].tex_index= tex_index;
//
//     r->triangle_count++;
//
//     //Workaround for overflowing the triangle buffer
//     if(r->triangle_count == MAX_TRIANGLES || tex_index == INVALID_TEX_INDEX) {
//         render_end_frame(r);
//         render_begin_frame(r);
//
//         // render_push_triangle(r, a, b, c, a_colour, b_colour, c_colour, a_uv, b_uv, c_uv, texture);
//     }
// }

void render_push_quad(renderer *r, vector2 coords[4], vector4 colours[4], vector2 uv[4], uint32_t texture) {
    uint32_t tex_index = INVALID_TEX_INDEX;
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

    if(r->vertex_count + VERTICES_PER_QUAD >= MAX_VERTICES || r->index_count + INDECIES_PER_QUAD >= MAX_INDECIES || tex_index == INVALID_TEX_INDEX) {
        render_end_frame(r);
        render_begin_frame(r);
    }

    uint32_t base_index = r->vertex_count;

    for(int i = 0; i < VERTICES_PER_QUAD; i++) {
        r->vertex_data[r->vertex_count++] = (render_vertex){coords[i], colours[i], uv[i], tex_index};
    }

    //First triangle
    r->index_data[r->index_count++] = base_index;
    r->index_data[r->index_count++] = base_index + 1;
    r->index_data[r->index_count++] = base_index + 2;

    //Second triangle
    r->index_data[r->index_count++] = base_index + 2;
    r->index_data[r->index_count++] = base_index + 1;
    r->index_data[r->index_count++] = base_index + 3;
}

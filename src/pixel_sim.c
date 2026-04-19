#include "../include/pixel_sim.h"
#include "../externals/glad/glad.h"
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/list.h"
#include "../include/input.h"
#include "../include/rigidbody.h"
#include "../include/shader.h"

/*GPU Resources*/
GLuint g_matter_in_ssbo;
GLuint g_matter_out_ssbo;
GLuint g_canvas_tex;
GLuint g_color_prog;   /* compute: write colours to canvas_img */
GLuint g_fall_prog;    /* compute: vertical fall pass          */
GLuint g_slide_prog;   /* compute: diagonal slide pass         */
GLuint g_render_prog;  /* vert + frag                          */
GLuint g_quad_vao;

/*Counters passed in as uniforms*/
unsigned int g_sim_step = 0;
unsigned int g_move_step = 0;

/*Dynamic Settings*/
float g_brush_radius = 1.0f;
unsigned int g_draw_matter_id = MATTER_SAND;
int g_is_paused = 0;

double prev_mx = -1.0, prev_my = -1.0;

pixel variant_colours[NUM_PIXEL_TYPES] = {
    {0x00, 0x00, 0x00, 0x00}, //NONE
    {0xd6, 0xcd, 0x18, 0xff}, //SAND
    {0x34, 0x34, 0xeb, 0xff}, //WATER
    {0x6e, 0x31, 0x0d, 0xff}, //WOOD
    {0x69, 0x67, 0x65, 0xff}, //STONE
};

uint32_t pack_matter(uint8_t r, uint8_t g, uint8_t b, uint8_t id) {
    uint32_t colour = (r << 16) | (g << 8) | b;
    return (colour << 8) | (id & 0xFFu);
}

uint32_t matter_default_colour(uint8_t id) {
    switch(id) {
        case MATTER_SAND: return pack_matter(0xc2, 0xb2, 0x80, MATTER_SAND);
        case MATTER_WOOD: return pack_matter(0xba, 0x8c, 0x63, MATTER_WOOD);
        default: return 0u; //Empty
    }
}

int inside_grid(int x, int y) {
    return x >= 0 && x < CANVAS_SIZE_X && y >= 0 && y < CANVAS_SIZE_Y;
}

int grid_index(int x, int y) {return y * CANVAS_SIZE_X + x;}

void draw_circle(ivector2 c, float radius, uint8_t m_id) {
    uint32_t packed = matter_default_colour(m_id);
    int i_r = (int)radius;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_matter_in_ssbo);
    for(int y = c.y - i_r; y <= c.y + i_r; y++) {
        for(int x = c.x - i_r; x <= c.x + i_r; x++) {
            float dx = (float)(x - c.x);
            float dy = (float)(y - c.y);

            if(roundf(sqrtf(dx*dx + dy*dy)) <= radius && inside_grid(x, y)) {
                GLintptr offset = (GLintptr)(grid_index(x, y) * sizeof(unsigned int));
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(unsigned int), &packed);
            }
        }
    }
}

void bresenham(ivector2 a, ivector2 b, float radius, uint8_t m_id) {
    int dx = abs(b.x - a.x);
    int dy = abs(b.y - a.y);

    int sx = (a.x < b.x) ? 1 : -1;
    int sy = (a.y < b.y) ? 1 : -1;

    int err = dx-dy;
    int cx = a.x;
    int cy = a.y;

    while(1){
        ivector2 p = (ivector2){cx, cy};
        draw_circle(p, radius, m_id);
        if(cx == b.x && cy == b.y) break;

        int e2 = 2 * err;
        if(e2 >- dy) {
            err -= dy;
            cx += sx;
        }
        if(e2 < dx){
            err += dx;
            cy += sy;
        }
    }
}

void draw_matter_line(ivector2 a, ivector2 b) {
    bresenham(a, b, g_brush_radius, g_draw_matter_id);
}

void dispatch_compute(GLuint prog, int do_swap) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_matter_in_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_matter_out_ssbo);
    glBindImageTexture(2, g_canvas_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    glUseProgram(prog);
    glUniform1ui(glGetUniformLocation(prog, "u_sim_step"),  g_sim_step);
    glUniform1ui(glGetUniformLocation(prog, "u_move_step"), g_move_step);

    glDispatchCompute(NUM_WORK_GROUPS_X, NUM_WORK_GROUPS_Y, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    if(do_swap) {
        GLuint tmp = g_matter_in_ssbo;
        g_matter_in_ssbo = g_matter_out_ssbo;
        g_matter_out_ssbo = tmp;
        g_move_step++;
    }
}

void simulate(int is_paused) {
    if(!is_paused) {
        dispatch_compute(g_fall_prog, 1);
        dispatch_compute(g_slide_prog, 1);
    }

    dispatch_compute(g_color_prog, 0);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    g_sim_step++;
}

void render(void) {
    glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(g_render_prog);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_canvas_tex);
    glUniform1i(glGetUniformLocation(g_render_prog, "canvas_tex"), 0);
    glUniform1i(glGetUniformLocation(g_render_prog, "flip_y"), 1);
    glBindVertexArray(g_quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void initialise_gpu_sim(void) {
    g_color_prog = load_shader((shader_data[1]){"../data/shaders/colour.glsl", GL_COMPUTE_SHADER}, 1);
    glUseProgram(g_color_prog);
    GLint loc = glGetUniformLocation(g_color_prog, "canvas_size_x");
    glUniform1i(loc, CANVAS_SIZE_X);
    loc = glGetUniformLocation(g_color_prog, "canvas_size_y");
    glUniform1i(loc, CANVAS_SIZE_Y);
    g_fall_prog = load_shader((shader_data[1]){"../data/shaders/fall_empty.glsl", GL_COMPUTE_SHADER}, 1);
    glUseProgram(g_fall_prog);
    loc = glGetUniformLocation(g_fall_prog, "canvas_size_x");
    glUniform1i(loc, CANVAS_SIZE_X);
    loc = glGetUniformLocation(g_fall_prog, "canvas_size_y");
    glUniform1i(loc, CANVAS_SIZE_Y);
    g_slide_prog = load_shader((shader_data[1]){"../data/shaders/slide_down_empty.glsl", GL_COMPUTE_SHADER}, 1);
    glUseProgram(g_slide_prog);
    loc = glGetUniformLocation(g_slide_prog, "canvas_size_x");
    glUniform1i(loc, CANVAS_SIZE_X);
    loc = glGetUniformLocation(g_slide_prog, "canvas_size_y");
    glUniform1i(loc, CANVAS_SIZE_Y);
    g_render_prog = load_shader((shader_data[2]){(shader_data){"../data/shaders/colour_vert.vert", GL_VERTEX_SHADER}, (shader_data){"../data/shaders/colour_frag.frag", GL_FRAGMENT_SHADER}}, 2);

    glGenBuffers(1, &g_matter_in_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_matter_in_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, CANVAS_SIZE_X * CANVAS_SIZE_Y * sizeof(unsigned int), NULL, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &g_matter_out_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_matter_out_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, CANVAS_SIZE_X * CANVAS_SIZE_Y * sizeof(unsigned int), NULL, GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &g_canvas_tex);
    glTextureStorage2D(g_canvas_tex, 1, GL_RGBA8, CANVAS_SIZE_X, CANVAS_SIZE_Y);
    glBindImageTexture(0, g_canvas_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
    glTextureParameteri(g_canvas_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(g_canvas_tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glBindTexture(GL_TEXTURE_2D, 0);

    glGenVertexArrays(1, &g_quad_vao);
}

ivector2 window_to_canvas(double x, double y) {
    return (ivector2){(int)round((x/800.0) * CANVAS_SIZE_X), (int)round((y/600.0) * CANVAS_SIZE_Y)};
}

void update_gpu_sim(void) {
    if(handler->key_status[KEY_MINUS] == KEY_JUST_PRESSED)
        g_brush_radius = fmaxf(g_brush_radius-1.0f, 1.0f);
    if(handler->key_status[KEY_PLUS] == KEY_JUST_PRESSED)
        g_brush_radius = fminf(g_brush_radius+1.0f, 10.0f);
    if(handler->key_status[KEY_0] == KEY_JUST_PRESSED) g_draw_matter_id = MATTER_EMPTY;
    if(handler->key_status[KEY_1] == KEY_JUST_PRESSED) g_draw_matter_id = MATTER_SAND;
    if(handler->key_status[KEY_2] == KEY_JUST_PRESSED) g_draw_matter_id = MATTER_WOOD;

    if(handler->key_status[MOUSE_BUTTON_LEFT] == KEY_JUST_PRESSED || handler->key_status[MOUSE_BUTTON_LEFT] == KEY_PRESSED && handler->mouseX >= 0.0) {
        ivector2 cur = window_to_canvas(handler->mouseX, handler->mouseY);
        ivector2 prev = (prev_mx >= 0) ? window_to_canvas(prev_mx, prev_my) : cur;
        draw_matter_line(prev, cur);
        prev_mx = handler->mouseX;
        prev_my = handler->mouseY;
    }
    if(handler->key_status[MOUSE_BUTTON_LEFT] == KEY_JUST_RELEASED) {
        prev_mx = -1.0;
        prev_my = -1.0;
    }

    simulate(0);
}

void update_pixel(world_grid *og, world_grid *ng, size_t index, int dir) {
    size_t new_index = index;
    size_t bottom = index + og->width;
    size_t left = index + og->width-1;
    size_t right = index + og->width+1;

    if((index / og->width) < og->height - 1) {
        if(og->data[bottom].type_variant == 0 && ng->data[bottom].type_variant == 0) {
            new_index = bottom;
        }
        else {
            if(dir) {
                if(index % og->width != og->width-1 && og->data[right].type_variant == 0 && ng->data[right].type_variant == 0) {
                    new_index = right;
                }
                else if(index % og->width != 0 && og->data[left].type_variant == 0 && ng->data[left].type_variant == 0) {
                    new_index = left;
                }
            }
            else {
                if(index % og->width != 0 && og->data[left].type_variant == 0 && ng->data[left].type_variant == 0) { new_index = left;
                }
                else if(index % og->width != og->width-1 && og->data[right].type_variant == 0 && ng->data[right].type_variant == 0) {
                    new_index = right;
                }
            }
        }
    }

    //So we don't overwrite existing cells
    if(ng->data[new_index].type_variant == 0) {
        ng->data[new_index].type_variant = PIXEL_SAND;
    }
}

/**
 * Erases pixels in a square area. Used for testing the pixel destruction system
 * @param radius the half-width of the square erasure area
 * @param x the x-coordinate of the centre of the erasing square
 * @param y the y-coordinate of the centre of the erasing square
 * @param grid the world grid that erasure is occuring on
 * @param rbs a pointer to a list to store the coordintes of erased pixels that are part of a rigidbody
 */
void erase_pixels(int radius, int x, int y, world_grid *grid, list *rb_pts) {
    if (radius > 0) { //If the square is not one pixel in side length
        for (int h = 0; h < radius * 2; h++) {
            for (int w = 0; w < radius * 2; w++) {
                int dx = radius - w; // horizontal offset
                int dy = radius - h; // vertical offset
                //If the offset is in the grid and circle boundaries
                if((dx * dx + dy * dy) < (radius * radius) && (x + dx < grid->width) && (x + dx > -1) && (y + dy < grid->height) && (y + dy > -1)) {
                    //Set the pixel at this grid position to be dataless as its erased
                    grid->data[(y + dy) * grid->width + (x + dx)].type_variant = 0;
                    //If the pixel has a rigidbody parent, add it to rb_pts
                    if(grid->parents[(y + dy) * grid->width + (x + dx)] > 0) {
                        ivector2 new_pos = (ivector2){x+dx, y+dy};
                        push_value(rb_pts, ivector2, new_pos);
                    }
                }
            }
        }
    }
    else { //The erasure square is only one pixel in size
        //Set the pixel at this grid position to be dataless
        grid->data[(y  * grid->width) + x ].type_variant = 0;
        //If the pixel has a rigidbody parent, add it to rb_pts
        if(grid->parents[(y * grid->width) + x] > 0) {
            ivector2 new_pos = (ivector2){x, y};
            push_value(rb_pts, ivector2, new_pos);
        }
    }
}


void erase_pixels_callback(pixel_func_args *args) {
    //Do pixel erasure after updating all of the grid pixel data
    double cursor_x = handler->mouseX;
    double cursor_y = handler->mouseY;
    world_grid *og = args->gbuf->grids[args->gbuf->curr];
    add_pixel_func_args *ea = args->extra_data;
    plaza *p = (plaza *) ea->p;
    // plaza *p = (plaza *)args->extra_data;

    list *point_list = list_alloc(10, sizeof(ivector2)); //List to store coordinates of points which have been erased
    list *entity_list = list_alloc(10, sizeof(int32_t)); //List to store all of the parent entities of these rigidbodies (to prevent double rigidbody processing)
    erase_pixels(ea->scale, cursor_x * 1/(float)PIXEL_SIZE, cursor_y * 1/(float)PIXEL_SIZE, og, point_list); //Erase pixels at the current cursor position

    if(point_list->size > 0)
        printf("========= NEW ERASURE ========\n");
    for(int i = 0; i < point_list->size; i++) { //For each erased point which has a rigidbody parent
        ivector2 point = get_value(point_list, ivector2, i);
        uint32_t grid_pos = (point.y * og->width) + point.x; //Get its grid index

        //Checking if we have already processed a particular rigidbody by checking if we have stored the id of its parent entity
        int index = -1;
        for(int i = 0; i < entity_list->size; i++) {
            if(get_value(entity_list, int32_t, i) == og->parents[grid_pos]){
                index = i;
                break;
            }
        }
        //If not, mark it as processed
        if(index == -1) {
            push_value(entity_list, int32_t, og->parents[grid_pos]);
        }

        rigidbody *rb = get_component_from_entity(p, og->parents[grid_pos], RIGIDBODY); //Get a reference to the actual rigidbody struct
        transform *t = get_component_from_entity(p, og->parents[grid_pos], TRANSFORM); //And to its corresponding transform

        //Get the relative position of the pixel to the rigidbody that is stored in the rigidbody struct
        vector2 d = {(point.x + 0.5) - t->position.x, (point.y + 0.5f) - t->position.y};
        vector2 rotated_pos = rotate_about_point(&d, &(vector2){0,0}, -t->rotation, 1);

        //Computign half-lengths of this rigidbody
        float half_width = (rb->width - 1) * 0.5f;
        float half_height = (rb->height - 1) * 0.5f;

        //Get the pixel's position in the rigidbody
        int ix = (int)floorf(rotated_pos.x + half_width + 0.5f);
        int iy = (int)floorf(rotated_pos.y + half_height + 0.5f);

        //If the unrotated pixel is in the rigidbody's bounding box and the mask says its not erased, colour it
        if(ix >= 0 && ix < rb->width && iy >= 0 && iy < rb->height && rb->mask[iy * rb->width + ix]) {
            //Clear mask. Must be done before removing pixel
            rb->mask[iy * rb->width + ix] = 0;

            for(int j = 0; j < rb->pixel_count; j++) {
                //Pixel coords relative to rigidbody center
                vector2 rel = rb->pixel_coords[j];

                //Rotate to world space
                vector2 world_pos = rotate_about_point(&rel, &(vector2){0,0}, t->rotation, 1);
                world_pos.x += t->position.x;
                world_pos.y += t->position.y;

                //Compare to the erased pixel using a small epsilon
                float dx = world_pos.x - ((float)point.x + 0.5f);
                float dy = world_pos.y - ((float)point.y + 0.5f);

                if(dx*dx + dy*dy < 0.25f) { //within half a pixel
                    rb->pixel_coords[j] = rb->pixel_coords[rb->pixel_count - 1]; //remove pixel by swapping with last index
                    rb->pixel_count--; //Reduce pixel count
                    break;
                }
            }
        }
        og->parents[grid_pos] = -1; //Need to actually set the pixel to have no parent otherwise bfs will be wierd
    }
    //Split all processed rigidbodies
    for(int i = 0; i < entity_list->size; i++) {
        printf("================ NEW SPLIT ==============\n");
        split_rigidbody(get_value(entity_list, int32_t, i), p, og, world_id);
    }

    free(ea);
    free_list(point_list);
    free_list(entity_list);
}

void add_pixel_callback(pixel_func_args *args) {
    world_grid *og = args->gbuf->grids[args->gbuf->curr];
    add_pixel_func_args *ea = (add_pixel_func_args *)args->extra_data;
    int scale = ea->scale;
    int x = args->cursor_pos.x;
    int y = args->cursor_pos.y;

    if(scale > 1) {
        for(int h = 0; h < scale; h++) {
            for(int w = 0; w < scale; w++) {
                int dx = scale - w; // horizontal offset
                int dy = scale - h; // vertical offset
                int idx = (y + dy) * og->width + (x + dx);
                //If the offset is in the grid boundary and the cell at this offset is not occupied
                if((dx * dx + dy * dy) < (scale * scale) && (x + dx < og->width)
                    && (x + dx > -1)  && (y + dy < og->height) && (y + dy > -1)
                    && og->data[idx].type_variant == PIXEL_NONE && og->parents[idx] == 0) {
                    //Set the pixel at this grid position to be the variant specified
                    og->data[(y + dy) * og->width + (x + dx)].type_variant = ea->type_variant;
                }
            }
        }
    }
    else {
        int idx = args->cursor_pos.y * og->width + args->cursor_pos.x;

        if(og->data[idx].type_variant == PIXEL_NONE && og->parents[idx] == 0) {
            og->data[idx].type_variant = ea->type_variant;
        }
    }
    free(ea);
}


/**
 * Initialises a world_grid to be blank
 * @param width the width of the grid
 * @param height the height of the grid
 * @return a pointer to the created world_grid on the heap
 */
world_grid *initialise_grid(uint32_t width, uint32_t height) {
    world_grid *grid = malloc(sizeof(world_grid));
    grid->height = height;
    grid->width = width;
    grid->pixels = calloc(width * height, sizeof(pixel)); //All pixels are initially 0
    grid->parents = calloc(width * height, sizeof(uint32_t));
    grid->data = calloc(width * height, sizeof(pixel_data));
    return grid;
}

/**
 * Clears the pixels and parents buffers of a world_grid, setting the former to 0 and the latter to -1
 * @param grid the grid to clear
 */
void clear_grid(world_grid *grid) {
    memset(grid->pixels, 0, sizeof(uint8_t) * grid->width * grid->height * 4);
    memset(grid->parents, 0, sizeof(uint32_t) * grid->width * grid->height);
    memset(grid->data, 0, sizeof(pixel_data) * grid->width * grid->height);
}

/**
 * Frees a world_grid alongside its pixels and parents buffers
 * @param grid the grid to free
 */
void free_grid(world_grid *grid) {
    free(grid->parents);
    free(grid->pixels);
    free(grid);
}

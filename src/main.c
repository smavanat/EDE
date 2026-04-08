#define _XOPEN_SOURCE 500 //Needed to get deprecated functions
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L  // must come first
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "../externals/glad/glad.h"
#include "../externals/GLFW/glfw3.h"
#include "../include/stb_image.h"
#include "../include/basic_systems.h"
#include "../include/world.h"
#include "../include/renderer.h"
#include "../include/rigidbody.h"

#define FRAME_RATE 1000 / 60.0f

world *w;
renderer *gRenderer;
pixel_renderer *pRenderer;
debug_renderer *dRenderer;
b2WorldId world_id;
GLFWwindow *gw = NULL;
world_grid *grid = NULL;

//TODO: Need to make the list in the world that holds all the systems a priority queue so we can order the systems properly
//      If a rigidbody drops to one pixel, just delete that rigidbody and treat the pixel as part of the pixel simulation
//      Should just make two world grid buffers that we swap instead of constantly mallocing new ones
//      Fix bug where erasing a dynamic box2d body also erases from the opposite side
//NOTE: ALL RIGIDBODIES NEED TO HAVE EVEN DIMENSIONS TO ENSURE WE DON'T GET WEIRD HALF-PIXEL OFFSETS

//Thank you Bernardo: https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
#ifdef WIN32
#include <windows.h>
#else
#include <time.h>   // for nanosleep
#include <unistd.h> // for usleep
#endif

/**
 * Cross-platform sleep function
 * @param milliseconds the time the thread should sleep for in milliseconds
 */
void sleep_ms(int milliseconds){
#ifdef WIN32
    Sleep(milliseconds);
#else
#if defined(_POSIX_VERSION) && (POSIX_VERSION >= 199309L)
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    if (milliseconds >= 1000)
      sleep(milliseconds / 1000);
    usleep((milliseconds % 1000) * 1000);
#endif
#endif
}

/**
 * Callback function to adjust the outputs of various renderers on window resizing
 * @param window a pointer to the GLFWindow that represents the current window
 * @param width the new window width
 * @param height the new window height
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    //Should really figure out how we can combine the renderers
    //Projection matrix for normal renderer
    glm_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f, gRenderer->projection);
    int proj_loc = glGetUniformLocation(gRenderer->shader, "uProjection");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (float *)gRenderer->projection);

    //Projection matrix for pixel renderer
    glm_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f, pRenderer->projection);
    proj_loc = glGetUniformLocation(pRenderer->shader, "uProjection");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (float *)pRenderer->projection);

    //Projection matrix for debug renderer
    glm_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f, dRenderer->projection);
    proj_loc = glGetUniformLocation(dRenderer->shader, "uProjection");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (float *)dRenderer->projection);
}

/**
 * Handles user input for a specific window
 * @param window the window we want to handle input for
 */
void process_input(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }
}

/**
 * Initialises the program and the window we are using for it
 * @param window a pointer to the GLFWwindow struct that holds a reference to the window we will initialise
 * @return success (1) or failure (0)
 */
int init(GLFWwindow **window) {
    //Initialising GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //For MacOS
    #endif

    //Initialising the window
    *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "EDE", NULL, NULL);
    if(window == NULL) {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);

    //Loading GLAD
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialise GLAD");
        return 0;
    }

    //Initialising the texture renderer
    gRenderer = malloc(sizeof(renderer));
    render_init(gRenderer, "../data/shaders/shader.vert", "../data/shaders/shader.frag");
    if(!gRenderer){
        printf("Failed to initialise the renderer\n");
        return 0;
    }

    //Initialising the pixel renderer
    pRenderer = malloc(sizeof(pixel_renderer));
    pixel_render_init(pRenderer, "../data/shaders/pixel_shader.vert", "../data/shaders/pixel_shader.frag");
    if(!pRenderer){
        printf("Failed to initialise the renderer\n");
        return 0;
    }

    //Initialising the debug renderer
    dRenderer = malloc(sizeof(debug_renderer));
    debug_render_init(dRenderer, "../data/shaders/debug_shader.vert", "../data/shaders/debug_shader.frag");
    if(!dRenderer){
        printf("Failed to initialise the debug renderer\n");
        return 0;
    }

    //Creating the world for physics
    b2WorldDef world_def = b2DefaultWorldDef();
    world_def.gravity = (vector2){ 0.0f, 1.0f };
    world_id = b2CreateWorld(&world_def);

    w = world_alloc(world_id);
    add_system(w, &render_system_init, &render_system_update); //Creating the render system
    add_system(w, &physics_system_init, &physics_system_update); //Creating the physics system -> Needs to update before rigidbodies otherwise colliders appear to 'accelerate' ahead of rigidbodies
    add_system(w, &rigidbody_system_init, &rigidbody_system_update); //Creating the rigidbody system

    world_init(w);

    //Allocating the world pixel grid
    grid = malloc(sizeof(world_grid));
    grid->height = PIXEL_SCREEN_HEIGHT;
    grid->width = PIXEL_SCREEN_WIDTH;
    grid->pixels = malloc(sizeof(pixel) * grid->height * grid->width);
    //Setting all of the pixels to be colourless and have no rigidbody parent by default
    for(int i = 0; i < grid->height * grid->width; i++) {
        memcpy(grid->pixels[i].colour, (uint8_t[]){0,0,0,0}, sizeof(grid->pixels[i].colour));
        grid->pixels[i].parent_body = -1;
    }

    return 1;
}

/**
 * Loads relevant program data and initialises the entities that use it
 * @return success(1) or failure (0)
 */
int load(void) {
    //Creating the entity to store the sprite component
    entity e = create_entity(w->p);
    sprite *spr = create_sprite(render_texture_load("../data/assets/container.jpg"),
                                (vector2[4]){{0.75f * SCREEN_WIDTH, 0.75f * SCREEN_HEIGHT}, {0.75f * SCREEN_WIDTH, 0.25f * SCREEN_HEIGHT}, {0.25f * SCREEN_WIDTH, 0.25f * SCREEN_HEIGHT}, {0.25f * SCREEN_WIDTH, 0.75f * SCREEN_HEIGHT}},
                                (vector4[4]){{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}},
                                (vector2[4]){{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}});
    add_component_to_entity(w->p, e, SPRITE, spr);

    //Entity to hold the falling box
    entity r = create_entity(w->p);
    transform *t = create_transform((vector2){10, 10}, 0, 0);
    add_component_to_entity(w->p, r, TRANSFORM, t);

    rigidbody *rb = create_rigidbody(r, 10, 6, (uint8_t[]){0xff, 0x00, 0x00, 0xff}, (t->position), grid);
    add_component_to_entity(w->p, r, RIGIDBODY, rb);

    collider *c = malloc(sizeof(collider));
    c->type = BOX;
    c->collider_id = create_box_collider(t->position, rb->width, rb->height, t->rotation + (M_PI), w->world_id, b2_dynamicBody);
    add_component_to_entity(w->p, r, COLLIDER, c);

    //Entity to hold the bottom platform
    entity base = create_entity(w->p);
    transform *tb = create_transform((vector2){40, 57}, 0, 0);
    add_component_to_entity(w->p, base, TRANSFORM, tb);

    rigidbody *rbb = create_rigidbody(base, 80, 6, (uint8_t[]){0x00, 0x00, 0x00, 0xff}, (tb->position), grid);
    add_component_to_entity(w->p, base, RIGIDBODY, rbb);

    collider *cb = malloc(sizeof(collider));
    cb->type = BOX;
    cb->collider_id = create_box_collider(tb->position, rbb->width, rbb->height, tb->rotation, w->world_id, b2_staticBody);
    add_component_to_entity(w->p, base, COLLIDER, cb);

    sys_query(w);
    return 1;
}

/**
 * Entrypoint of the program
 */
int main(int argc, char** argv) {
    // GLFWwindow *window = NULL;
    struct timeval stop, start; //Store the start and end times of a frame
    float dt = 0.0f; //Holds the time passed between frames

    if(init(&gw)) {
        printf("Initialised\n");
        if(load()) {
            printf("Loaded\n");
            //Render loop
            while(!glfwWindowShouldClose(gw)) {
                gettimeofday(&start, NULL);
                //input
                process_input(gw);

                glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                // printf("\n\n");
                // printf("================ NEW FRAME ==============\n");

                //Update the world
                world_update(w, dt);
                //Draw the pixel grid
                render_begin_pixel_frame(pRenderer);
                for(int i = 0; i < grid->height * grid->width; i++) {
                    draw_pixel(pRenderer, i, grid->pixels[i].colour);
                }
                render_end_pixel_frame(pRenderer);
                b2World_Step(w->world_id, 1.0f/60.0f, 4); //Update the physics

                //Need to draw a quad representing the erasure area
                double cursor_x, cursor_y;
                glfwGetCursorPos(gw, &cursor_x, &cursor_y);
                vector2 erasing_dimensions[] = {(vector2){cursor_x - (1 * PIXEL_SIZE), cursor_y - (2 * PIXEL_SIZE)}, (vector2){cursor_x - (1 * PIXEL_SIZE), cursor_y + (2 * PIXEL_SIZE)}, (vector2){cursor_x + (3 * PIXEL_SIZE), cursor_y + (2 * PIXEL_SIZE)}, (vector2){cursor_x + (3 * PIXEL_SIZE), cursor_y - (2 * PIXEL_SIZE)}};
                render_draw_quad(dRenderer, erasing_dimensions, (vector4){98 / 256.0, 17 / 256.0, 156 / 256.0, 1.0});
                debug_render_flush(dRenderer);

                //check and call events and swap the buffers
                glfwSwapBuffers(gw);
                glfwPollEvents();

                gettimeofday(&stop, NULL); //Get the time at the end of the frame
                dt = (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000.0f; //Get the frame length
                // If the frame took less time than the set frame rate, wait until the time is the frame rate
                if(dt < FRAME_RATE) {
                    sleep_ms((int)FRAME_RATE - dt);
                    dt = FRAME_RATE;
                }
            }

            //Program cleanup
            render_free(gRenderer);
            debug_render_free(dRenderer);
            pixel_render_free(pRenderer);

            glfwTerminate();
            return 0;
        }
    }
    return 1;
}

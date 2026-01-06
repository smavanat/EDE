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

#define FRAME_RATE 1000 / 60.0f

world *w;
renderer *gRenderer;
pixel_renderer *pRenderer;
debug_renderer *dRenderer;
b2WorldId world_id;

//TODO: Need to make the list in the world that holds all the systems a priority queue so we can order the systems properly

//Thank you Bernardo: https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
#ifdef WIN32
#include <windows.h>
#else
#include <time.h>   // for nanosleep
#include <unistd.h> // for usleep
#endif

void sleep_ms(int milliseconds){ // cross-platform sleep function
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

void process_input(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }
}

int init(GLFWwindow **window) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //For MacOS

    *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Learn OpenGL", NULL, NULL);
    if(window == NULL) {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialise GLAD");
        return 0;
    }

    gRenderer = malloc(sizeof(renderer));
    render_init(gRenderer, "../data/shaders/shader.vert", "../data/shaders/shader.frag");
    if(!gRenderer){
        printf("Failed to initialise the renderer\n");
        return 0;
    }

    pRenderer = malloc(sizeof(pixel_renderer));
    pixel_render_init(pRenderer, "../data/shaders/pixel_shader.vert", "../data/shaders/pixel_shader.frag");
    if(!pRenderer){
        printf("Failed to initialise the renderer\n");
        return 0;
    }

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

    w = world_alloc();
    add_system(w, &render_system_init, &render_system_update); //Creating the render system
    add_system(w, &rigidbody_system_init, &rigidbody_system_update); //Creating the rigidbody system
    add_system(w, &physics_system_init, &physics_system_update); //Creating the physics system

    world_init(w);

    return 1;
}

int load(void) {
    //Creating the entity to store the sprite component
    entity e = create_entity(w->p);
    sprite *spr = malloc(sizeof(sprite));
    spr->texture = render_texture_load("../data/assets/container.jpg");
    memcpy(spr->colours, (vector4[4]){{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}}, sizeof(spr->colours));
    memcpy(spr->coords, (vector2[4]){{0.75f * SCREEN_WIDTH, 0.75f * SCREEN_HEIGHT}, {0.75f * SCREEN_WIDTH, 0.25f * SCREEN_HEIGHT}, {0.25f * SCREEN_WIDTH, 0.25f * SCREEN_HEIGHT}, {0.25f * SCREEN_WIDTH, 0.75f * SCREEN_HEIGHT}}, sizeof(spr->coords));
    memcpy(spr->uv, (vector2[4]){{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}}, sizeof(spr->uv));

    add_component_to_entity(w->p, e, SPRITE, spr);

    entity r = create_entity(w->p);
    transform *t = malloc(sizeof(transform));
    t->angle = 0.0f;
    t->zIndex = 0;
    t->position = (vector2){10, 10};
    add_component_to_entity(w->p, r, TRANSFORM, t);

    rigidbody *rb = malloc(sizeof(rigidbody));
    rb->height = 5;
    rb->width = 10;
    rb->pixels = malloc(sizeof(pixel *) * rb->width * rb->height);
    for(int i = 0; i < rb->width * rb->height; i++) {
        rb->pixels[i] = malloc(sizeof(pixel));
        rb->pixels[i]->position = i;
        rb->pixels[i]->colour[0] = 0xff;
        rb->pixels[i]->colour[1] = 0x00;
        rb->pixels[i]->colour[2] = 0x00;
        rb->pixels[i]->colour[3] = 0xff;
    }
    add_component_to_entity(w->p, r, RIGIDBODY, rb);

    collider *c = malloc(sizeof(collider));
    c->type = BOX;
    c->collider_id = createBoxCollider((vector2){t->position.x + (rb->width * 0.5), t->position.y + (rb->height * 0.5)}, rb->width, rb->height, t->angle, world_id, b2_dynamicBody);
    add_component_to_entity(w->p, r, COLLIDER, c);

    entity base = create_entity(w->p);
    transform *tb = malloc(sizeof(transform));
    tb->angle = 0.0f;
    tb->zIndex = 0;
    tb->position = (vector2){0, 55};
    add_component_to_entity(w->p, base, TRANSFORM, tb);

    rigidbody *rbb = malloc(sizeof(rigidbody));
    rbb->height = 1;
    rbb->width = 80;
    rbb->pixels = malloc(sizeof(pixel *) * rbb->width * rbb->height);
    for(int i = 0; i < rbb->width * rbb->height; i++) {
        rbb->pixels[i] = malloc(sizeof(pixel));
        rbb->pixels[i]->position = i;
        rbb->pixels[i]->colour[0] = 0x00;
        rbb->pixels[i]->colour[1] = 0x00;
        rbb->pixels[i]->colour[2] = 0x00;
        rbb->pixels[i]->colour[3] = 0xff;
    }
    add_component_to_entity(w->p, base, RIGIDBODY, rbb);

    collider *cb = malloc(sizeof(collider));
    cb->type = BOX;
    cb->collider_id = createBoxCollider((vector2){tb->position.x + (rbb->width * 0.5), tb->position.y + (rbb->height * 0.5)}, rbb->width, rbb->height, tb->angle, world_id, b2_staticBody);
    add_component_to_entity(w->p, base, COLLIDER, cb);

    sys_query(w);
    return 1;
}

int main(int argc, char** argv) {
    GLFWwindow *window = NULL;
    struct timeval stop, start;
    float dt = 0.0f;

    if(init(&window)) {
        printf("Initialised\n");
        if(load()) {
            printf("Loaded\n");
            //Render loop
            while(!glfwWindowShouldClose(window)) {
                gettimeofday(&start, NULL);
                //input
                process_input(window);

                glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                world_update(w, dt);
                // render_draw_point(dRenderer, (vector2){-1.0f, -1.0f}, (vector4){0.0f, 0.0f, 1.0f, 1.0f});
                // render_draw_line(dRenderer, (vector2){0.0f, 0.0f}, (vector2){100.0, 100.0}, (vector4){1.0f, 0.0f, 0.0f, 1.0f});
                // render_draw_quad(dRenderer, &(quad){10.0f, 10.0f, 100.0f, 100.0f}, (vector4){1.0f, 0.0f, 0.0f, 1.0f});
                // render_draw_circle(dRenderer, (vector2){50.0f, 50.0f}, 20.0f, (vector4){1.0f, 0.0f, 0.0f, 1.0f});
                debug_render_flush(dRenderer);
                b2World_Step(world_id, 1.0f/60.0f, 4);

                //check and call events and swap the buffers
                glfwSwapBuffers(window);
                glfwPollEvents();
                gettimeofday(&stop, NULL);
                dt = (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000.0f;
                if(dt < FRAME_RATE) {
                    sleep_ms((int)FRAME_RATE - dt);
                    dt = FRAME_RATE;
                }
            }

            render_free(gRenderer);
            debug_render_free(dRenderer);

            glfwTerminate();
            return 0;
        }
    }
}

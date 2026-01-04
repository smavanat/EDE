#include "../externals/glad/glad.h"
#include "../externals/GLFW/glfw3.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/stb_image.h"
#include "../include/basic_systems.h"
#include "../include/world.h"
#include "../include/renderer.h"
#include <string.h>

world *w;
ecs_system *renderSystem, *rigidbodySystem;
renderer *gRenderer;
pixel_renderer *pRenderer;
debug_renderer *dRenderer;

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

    w = world_alloc();
    renderSystem = malloc(sizeof(ecs_system));
    renderSystem->init_func = &render_system_init;
    renderSystem->update_func = &render_system_update;
    push_value(w->systems, ecs_system *, renderSystem);

    rigidbodySystem = malloc(sizeof(ecs_system));
    rigidbodySystem->init_func = &rigidbody_system_init;
    rigidbodySystem->update_func = &rigidbody_system_update;
    push_value(w->systems, ecs_system *, rigidbodySystem);

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

    sys_query(w);
    return 1;
}

int main(int argc, char** argv) {
    GLFWwindow *window = NULL;

    if(init(&window)) {
        printf("Initialised\n");
        if(load()) {
            printf("Loaded\n");
            //Render loop
            while(!glfwWindowShouldClose(window)) {
                //input
                process_input(window);

                glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                world_update(w, 0);
                // render_draw_point(dRenderer, (vector2){-1.0f, -1.0f}, (vector4){0.0f, 0.0f, 1.0f, 1.0f});
                // render_draw_line(dRenderer, (vector2){0.0f, 0.0f}, (vector2){100.0, 100.0}, (vector4){1.0f, 0.0f, 0.0f, 1.0f});
                // render_draw_quad(dRenderer, &(quad){10.0f, 10.0f, 100.0f, 100.0f}, (vector4){1.0f, 0.0f, 0.0f, 1.0f});
                // render_draw_circle(dRenderer, (vector2){50.0f, 50.0f}, 20.0f, (vector4){1.0f, 0.0f, 0.0f, 1.0f});
                // debug_render_flush(dRenderer);

                //check and call events and swap the buffers
                glfwSwapBuffers(window);
                glfwPollEvents();
            }

            render_free(gRenderer);
            debug_render_free(dRenderer);

            glfwTerminate();
            return 0;
        }
    }
}

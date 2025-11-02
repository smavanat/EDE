#include "../externals/glad/glad.h"
#include "../externals/GLFW/glfw3.h"
#include <stdio.h>
#include <stdlib.h>
#include "../include/shader.h"
#include "../include/stb_image.h"
#include "../externals/cglm/cglm.h"
#include "../include/basic_systems.h"
#include "../include/world.h"
#include "../include/renderer.h"
#include <string.h>

world *w;
ecs_system *renderSystem;
renderer *gRenderer;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
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

    *window = glfwCreateWindow(800, 600, "Learn OpenGL", NULL, NULL);
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
    render_init(gRenderer, "../data/shaders/shader.vs", "../data/shaders/shader.fs"); //DOES NOT INITIALISE THE RENDERER
    if(!gRenderer){
        printf("Failed to initialise the renderer\n");
        return 0;
    }

    w = world_alloc();
    renderSystem = malloc(sizeof(ecs_system));
    renderSystem->init_func = &render_system_init;
    renderSystem->update_func = &render_system_update;
    push_value(w->systems, ecs_system *, renderSystem);

    world_init(w);

    return 1;
}

int load(void) {
    //Creating the entity to store the sprite component
    entity e = create_entity(w->p);
    sprite *spr = malloc(sizeof(sprite));
    spr->texture = render_texture_load("../data/assets/container.jpg");
    memcpy(spr->colours, (vector4[4]){{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}}, sizeof(spr->colours));
    memcpy(spr->coords, (vector2[4]){{0.5f, 0.5f}, {0.5f, -0.5f}, {-0.5f, -0.5f}, {-0.5f, 0.5f}}, sizeof(spr->coords));
    memcpy(spr->uv, (vector2[4]){{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}}, sizeof(spr->uv));
    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    add_component_to_entity(w->p, e, SPRITE, spr);
    sys_query(w);
    return 1;
}

int main(int argc, char** argv) {
    GLFWwindow *window = NULL;
    // shader *s = NULL;
    // unsigned int VAO, texture;

    if(init(&window)) {
        printf("Initialised\n");
        if(load()) {
            printf("Loaded\n");
            //Render loop
            while(!glfwWindowShouldClose(window)) {
                //input
                process_input(window);

                render_begin_frame(gRenderer);
                //bind texture
                // glBindTexture(GL_TEXTURE_2D, texture);
                // // mat3 *transform;
                // // vec2 tv = (vec2){0.5f, -0.5f};
                // // glm_translate2d(transform, &tv);
                // // glm_rotate2d(transform, )
                world_update(w, 0);
                render_end_frame(gRenderer);

                //check and call events and swap the buffers
                glfwSwapBuffers(window);
                glfwPollEvents();
            }

            glfwTerminate();
            return 0;
        }
    }
}

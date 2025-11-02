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
    // spr->shader = load_shader("../data/shaders/shader.vs", "../data/shaders/shader.fs");
    // spr->VAO = 0;
    spr->texture = render_texture_load("../data/assets/container.jpg");
    memcpy(spr->colours, (vector4[4]){{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}}, sizeof(spr->colours));
    memcpy(spr->coords, (vector2[4]){{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}}, sizeof(spr->coords));
    memcpy(spr->colours, (vector4[4]){{0.5f, 0.5f}, {0.5f, -0.5f}, {-0.5f, -0.5f}, {-0.5f, 0.5f}}, sizeof(spr->uv));
    //
    // float vertices[] = {
    //     // positions          // colors           // texture coords
    //      0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
    //      0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    //     -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    //     -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    // };
    // unsigned int indices[] = {
    //     0, 1, 3, // first triangle
    //     1, 2, 3  // second triangle
    // };
    // unsigned int VBO, EBO;
    // glGenVertexArrays(1, &spr->VAO);
    // glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);
    //
    // glBindVertexArray(spr->VAO);
    //
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    //
    // // position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    // // color attribute
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);
    // // texture coord attribute
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // glEnableVertexAttribArray(2);
    //
    //
    // // load and create a texture
    // glGenTextures(1, &spr->texture);
    // glBindTexture(GL_TEXTURE_2D, spr->texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // // set the texture wrapping parameters
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // // set texture filtering parameters
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    // int width, height, nrChannels;
    // unsigned char *data = stbi_load("../data/assets/container.jpg", &width, &height, &nrChannels, 0);
    // if (data)
    // {
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // }
    // else
    // {
    //     printf("Failed to load texture\n");
    //     return 0;
    // }
    // stbi_image_free(data);
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

                //Rendering commands here
                // glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //Setting background colour
                // glClear(GL_COLOR_BUFFER_BIT);

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

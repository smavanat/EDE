#include "../externals/glad/glad.h"
#include "../externals/GLFW/glfw3.h"
#include <stdio.h>
#include <stdlib.h>
#include "../include/shader.h"
#include "../include/stb_image.h"
#include "../externals/cglm/cglm.h"
#include "../include/basic_systems.h"
#include "../include/world.h"

world *w;
ecs_system *renderSystem;

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

    w = world_alloc();
    renderSystem = malloc(sizeof(ecs_system));
    renderSystem->init_func = &render_init;
    renderSystem->update_func = &render_update;
    push_value(w->systems, ecs_system *, renderSystem);

    world_init(w);

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
    return 1;
}

int load(/*shader **s, unsigned int *VAO, unsigned int *texture*/void) {
    //Creating the entity to store the sprite component
    entity e = create_entity(w->p);
    sprite *spr = malloc(sizeof(sprite));
    spr->shader = load_shader("../data/shaders/shader.vs", "../data/shaders/shader.fs");
    spr->VAO = 0;
    spr->texture = 0;

    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, EBO;
    glGenVertexArrays(1, &spr->VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(spr->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // load and create a texture
    glGenTextures(1, &spr->texture);
    glBindTexture(GL_TEXTURE_2D, spr->texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
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
        if(load(/*&s, &VAO, &texture*/)) {
            printf("Loaded\n");
            // if(s == NULL){
            //     printf("s is null\n");
            // }
            // printf("Shader id: %i\n", s->id);

            //Render loop
            while(!glfwWindowShouldClose(window)) {
                //input
                process_input(window);

                //Rendering commands here
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //Setting background colour
                glClear(GL_COLOR_BUFFER_BIT);

                //bind texture
                // glBindTexture(GL_TEXTURE_2D, texture);
                // // mat3 *transform;
                // // vec2 tv = (vec2){0.5f, -0.5f};
                // // glm_translate2d(transform, &tv);
                // // glm_rotate2d(transform, )
                //
                // use(s);
                // glBindVertexArray(VAO);
                // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                world_update(w, 0);

                //check and call events and swap the buffers
                glfwSwapBuffers(window);
                glfwPollEvents();
            }

            glfwTerminate();
            return 0;
        }
    }
}

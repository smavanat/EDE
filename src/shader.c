#include "../include/shader.h"
#include <stdio.h>
#include "../include/file_utils.h"

/**
 * Creates a shader by loading its constituent vertex and fragment shaders
 * @param vertex_path the filepath to the vertex shader
 * @param fragment_path the filepath to the fragment shader
 * @return the id of the created shader
 */
shader load_shader(char *vertex_path, char *fragment_path) {
    char *vertex_buf;
    char *fragment_buf;
    int result;

    //Read the vertex shader into a buffer
    result = read_to_end(vertex_path, &vertex_buf, true);
    if(-1 == result) {
        printf("ERROR::VERTEX_SHADER::FILE_NOT_SUCCESSFULLY_READ\n");
        return NULL;
    }

    //Read the fragment shader into a buffer
    result = read_to_end(fragment_path, &fragment_buf, true);
    if(-1 == result) {
        printf("ERROR::FRAGMENT_SHADER::FILE_NOT_SUCCESSFULLY_READ\n");
        return NULL;
    }

    unsigned int vertex, fragment;
    char infoLog[512];

    //Create the vertex shader from the code in the glsl and compile it
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const char *const *)&vertex_buf, NULL);
    glCompileShader(vertex);

    //Print compile errors if any:
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);
    if(!result) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        for(int i = 0; i < 512; i++){
            printf("%c", infoLog[i]);
        }
        printf("\n");
    }

    //Create the fragment shader from the code in the glsl and compile it
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char *const *)&fragment_buf, NULL);
    glCompileShader(fragment);

    //Print compile errors if any:
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &result);
    if(!result) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
        for(int i = 0; i < 512; i++){
            printf("%c", infoLog[i]);
        }
        printf("\n");
    }

    //Create the shader and attach the created vertex and fragment shader
    shader s = glCreateProgram();
    glAttachShader(s, vertex);
    glAttachShader(s, fragment);
    glLinkProgram(s);

    //Print errors if any:
    glGetProgramiv(s, GL_LINK_STATUS, &result);
    if(!result) {
        glGetProgramInfoLog(s, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        for(int i = 0; i < 512; i++){
            printf("%c", infoLog[i]);
        }
        printf("\n");
    }

    //Cleanup
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return s;
}

/**
 * Calls the glUseProgram function if the current program used by OpenGL has a different id to the one passed in
 * @param shader the shader to use
 */
void use(shader shader) {
    static GLuint currentProgram = 0;

    if ((GLuint)currentProgram != shader)
        glUseProgram(shader);
}

void set_bool(shader s, char *name, bool value) {
    glUniform1i(glGetUniformLocation(s, name), (int)value);
}
void set_int(shader s, char *name, int value) {
    glUniform1i(glGetUniformLocation(s, name), value);
}
void set_float(shader s, char *name, float value) {
    glUniform1i(glGetUniformLocation(s, name), value);
}

#include "../include/shader.h"
#include <stdio.h>
#include "../include/file_utils.h"

shader load_shader(char *vertexPath, char *fragmentPath) {
    char *vertexBuf;
    char *fragmentBuf;
    int result;

    result = read_to_end(vertexPath, &vertexBuf, true);
    if(-1 == result) {
        printf("ERROR::VERTEX_SHADER::FILE_NOT_SUCCESSFULLY_READ\n");
        return NULL;
    }

    result = read_to_end(fragmentPath, &fragmentBuf, true);
    if(-1 == result) {
        printf("ERROR::FRAGMENT_SHADER::FILE_NOT_SUCCESSFULLY_READ\n");
        return NULL;
    }

    unsigned int vertex, fragment;
    char infoLog[512];

    //Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const char *const *)&vertexBuf, NULL);
    glCompileShader(vertex);

    //print compile errors if any:
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);
    if(!result) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        for(int i = 0; i < 512; i++){
            printf("%c", infoLog[i]);
        }
        printf("\n");
    }

    //Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char *const *)&fragmentBuf, NULL);
    glCompileShader(fragment);

    //print compile errors if any:
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &result);
    if(!result) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
        for(int i = 0; i < 512; i++){
            printf("%c", infoLog[i]);
        }
        printf("\n");
    }

    shader s = glCreateProgram();
    glAttachShader(s, vertex);
    glAttachShader(s, fragment);
    glLinkProgram(s);
    //print errors if any:
    glGetProgramiv(s, GL_LINK_STATUS, &result);
    if(!result) {
        glGetProgramInfoLog(s, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        for(int i = 0; i < 512; i++){
            printf("%c", infoLog[i]);
        }
        printf("\n");
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // shader* s = malloc(sizeof(shader));
    // s->id = id;
    return s;
}

void use(shader shader) {
    static GLuint currentProgram = 0;

    if ((GLuint)currentProgram != shader)
        glUseProgram(shader);

    // glUseProgram(shader);
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

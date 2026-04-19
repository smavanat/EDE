#include "../include/shader.h"
#include <stdint.h>
#include <stdio.h>
#include "../include/file_utils.h"

/**
 * Creates a shader by loading its constituent vertex and fragment shaders
 * @param sd an array of shader_data structs representing each shader to be loaded
 * @param num_shaders the number of shaders to be loaded
 * @return the id of the created shader
 */
shader load_shader(shader_data *sd, unsigned int num_shaders) {
    uint32_t shader_buf[num_shaders]; //Array to store the ids of the loaded shader sub-programs
    shader s = glCreateProgram();

    //Attaching all of the shaders together
    for(int i = 0; i < num_shaders; i++) {
        shader_buf[i] = create_shader(sd[i]);
        glAttachShader(s, shader_buf[i]);
    }

    glLinkProgram(s);
    int result;
    char infolog[512];

    //Print errors if any:
    glGetProgramiv(s, GL_LINK_STATUS, &result);
    if(!result) {
        glGetProgramInfoLog(s, 512, NULL, infolog);
        printf("ERROR::SHADER::LINKING_FAILED\n");
        for(int i = 0; i < 512; i++){
            if(infolog[i] == '\0') break;
            printf("%c", infolog[i]);
        }
        printf("\n");
    }

    //Cleanup
    for(int i = 0; i < num_shaders; i++) {
        glDeleteShader(shader_buf[i]);
    }

    return s;
}

/**
 * Creates an OpenGL shader object from a loaded glsl file
 * @param s a shader_data struct containg the path of the shader and the type of shader we want to load
 * @return the id of the loaded shader
 */
uint32_t create_shader(shader_data s) {
    char *buf; //Stores the data in the file
    int result; //For storing the results of various operations

    //Load file data
    result = read_to_end(s.shader_path, &buf, true);
    if(-1 == result) {
        printf("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n");
        printf("Path: ");
        int i = 0;
        while(s.shader_path[i] != '\0') {
            printf("%c", s.shader_path[i]);
            i++;
        }
        printf("\n");
        return NULL;
    }

    uint32_t sh;
    char infolog[512];

    //Create the vertex shader from the code in the glsl and compile it
    sh = glCreateShader(s.shader_type);
    glShaderSource(sh, 1, (const char *const *)&buf, NULL);
    glCompileShader(sh);

    //Print compile errors if any:
    glGetShaderiv(sh, GL_COMPILE_STATUS, &result);
    if(!result) {
        glGetShaderInfoLog(sh, 512, NULL, infolog);
        printf("ERROR::SHADER::COMPILATION_FAILED\n");
        printf("Path: ");
        int j = 0;
        while(s.shader_path[j] != '\0') {
            printf("%c", s.shader_path[j]);
            j++;
        }
        printf("\n");
        for(int i = 0; i < 512; i++){
            if(infolog[i] == '\0') break;
            printf("%c", infolog[i]);
        }
        printf("\n");
    }

    return sh;
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

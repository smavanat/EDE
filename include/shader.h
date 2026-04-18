#ifndef __SHADER_H__
#define __SHADER_H__
#include "../externals/glad/glad.h"
#include <stdbool.h>

typedef uint32_t shader;

#ifdef __cplusplus
extern "C" {
#endif

//Struct to store data about a shader that needs to be loaded
typedef struct {
    char *shader_path; //The path to the shader code
    int shader_type; //The type of shader this represents
} shader_data;

shader load_shader_old(char *vertexPath, char *fragmentPath);
shader load_shader(shader_data *sd, unsigned int num_shaders);
uint32_t create_shader(shader_data s);

void use(shader shader);

void set_bool(shader s, char *name, bool value);
void set_int(shader s, char *name, int value);
void set_float(shader s, char *name, float value);

#ifdef __cplusplus
}
#endif
#endif

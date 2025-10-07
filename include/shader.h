#ifndef __SHADER_H__
#define __SHADER_H__
#include "../externals/glad/glad.h"
#include <stdbool.h>

typedef uint32_t shader;

#ifdef __cplusplus
extern "C" {
#endif
shader load_shader(char *vertexPath, char *fragmentPath);

void use(shader shader);

void set_bool(shader s, char *name, bool value);
void set_int(shader s, char *name, int value);
void set_float(shader s, char *name, float value);

#ifdef __cplusplus
}
#endif
#endif

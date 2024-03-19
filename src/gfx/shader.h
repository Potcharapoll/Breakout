#ifndef SHADER_H
#define SHADER_H
#include "gfx.h"

struct Shader {
    GLuint handle, vs, fs;
};

struct Shader shader_load(char *vs_path, char *fs_path);
void shader_use(struct Shader self);
void shader_delete(struct Shader self);
#endif

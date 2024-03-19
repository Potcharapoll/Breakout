#ifndef TEXTURE_H
#define TEXTURE_H
#include "gfx.h"
#include <cglm/cglm.h>

struct Texture {
    GLuint handle;
    ivec2 size;
};

struct Texture texture_load(char *path);
void texture_destroy(struct Texture self);
void texture_bind(struct Texture self);
void texture_unbind(void);
#endif

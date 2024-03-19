#ifndef VAO_H
#define VAO_H
#include "gfx.h"
#include "../util/types.h"

struct VAO {
    GLuint handle;
};

struct VAO vao_create(void);
void vao_destroy(struct VAO self);
void vao_bind(struct VAO self);
void vao_unbind(void);
void vao_attr(u32 idx, u32 size, GLenum type, size_t stride, size_t offset);
#endif

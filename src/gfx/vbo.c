#include "vbo.h"
#include "gfx.h"

struct VBO vbo_create(GLenum type, bool dynamic) {
    struct VBO vbo = {.type = type, .dynamic = dynamic};
    glGenBuffers(1, &vbo.handle);
    return vbo;
}

void vbo_destroy(struct VBO self) {
    glDeleteBuffers(1, &self.handle);
}
void vbo_bind(struct VBO self) {
    glBindBuffer(self.type, self.handle);
}
void vbo_unbind(struct VBO self) {
    glBindBuffer(self.type, 0);
}

void vbo_data(struct VBO self, size_t size, void *data) {
    glBufferData(self.type, size, data, (self.dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

void vbo_subdata(struct VBO self, size_t size, void *data) {
    glBufferSubData(self.type, 0, size, data);
}

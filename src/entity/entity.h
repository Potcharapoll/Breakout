#ifndef RECT_H
#define RECT_H
#include <cglm/cglm.h>

struct Vertex {
    vec3 pos;
    vec3 color;
};

struct Entity {
    ivec2 size;
    float x, y;

    struct Vertex vertices[4];
};

struct Entity entity_init(ivec2 size, float x, float y, vec3 color);
void entity_vertices_set(struct Entity *self, ivec2 size, vec2 pos);
void entity_color_set(struct Entity *self, vec3 color);
#endif

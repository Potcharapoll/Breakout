#include "entity.h"

struct Entity entity_init(ivec2 size, float x, float y, vec3 color) {
    struct Entity entity = {
        .size = { size[0], size[1] },
        .x    = x,
        .y    = y 
    };

    entity_vertices_set(&entity, size, (vec2){x,y});
    entity_color_set(&entity, color);
    return entity;
}

void entity_vertices_set(struct Entity *self, ivec2 size, vec2 pos) {
    glm_vec3_copy((vec3){pos[0],         pos[1],         0.0}, self->vertices[0].pos);
    glm_vec3_copy((vec3){pos[0]+size[0], pos[1],         0.0}, self->vertices[1].pos);
    glm_vec3_copy((vec3){pos[0]+size[0], pos[1]+size[1], 0.0}, self->vertices[2].pos);
    glm_vec3_copy((vec3){pos[0],         pos[1]+size[1], 0.0}, self->vertices[3].pos);
}

void entity_color_set(struct Entity *self, vec3 color) {
    glm_vec3_copy(color, self->vertices[0].color);
    glm_vec3_copy(color, self->vertices[1].color);
    glm_vec3_copy(color, self->vertices[2].color);
    glm_vec3_copy(color, self->vertices[3].color);
}

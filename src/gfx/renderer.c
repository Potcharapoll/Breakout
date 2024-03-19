#include "renderer.h"
#include "gfx.h"
#include "shader.h"
#include "texture.h"
#include "vao.h"
#include "vbo.h"

#include "../util/defs.h"

#include "../entity/entity.h"

void renderer_init(struct Renderer *self) {
    self->vao = vao_create();
    self->vbo = vbo_create(GL_ARRAY_BUFFER, true);
    self->ibo = vbo_create(GL_ELEMENT_ARRAY_BUFFER, false);

    vao_bind(self->vao);
    vbo_bind(self->vbo);
    vbo_data(self->vbo, 4*sizeof(struct Vertex), NULL);
    vao_attr(0, 3, GL_FLOAT, sizeof(struct Vertex), offsetof(struct Vertex, pos));
    vao_attr(1, 3, GL_FLOAT, sizeof(struct Vertex), offsetof(struct Vertex, color));

    unsigned short indices[] = { 0, 1, 2, 2, 3, 0 };
    vbo_bind(self->ibo);
    vbo_data(self->ibo, sizeof(indices), indices);

    vao_unbind();
    vbo_unbind(self->vbo);
    vbo_unbind(self->ibo);

    self->texture = texture_load("../res/images/font.png");
    self->shaders[SHADER_BASIC] = shader_load("../res/shaders/basic.vs", "../res/shaders/basic.fs");
    self->shaders[SHADER_TEXT] = shader_load("../res/shaders/text.vs", "../res/shaders/text.fs");

    glm_ortho(0, WIDTH, 0, HEIGHT, 0.0f, 1.0f, self->proj);

    renderer_shader_use(self, SHADER_BASIC);
    glUniformMatrix4fv(glGetUniformLocation(self->shader.handle, "proj"), 1, GL_FALSE, &self->proj[0][0]);

    renderer_shader_use(self, SHADER_TEXT);
    glUniformMatrix4fv(glGetUniformLocation(self->shader.handle, "proj"), 1, GL_FALSE, &self->proj[0][0]);

}

void renderer_destroy(struct Renderer *self) {
    for (int i = 0; i < SHADER_LAST; i++) {
        shader_delete(self->shaders[i]);
    } 

    texture_destroy(self->texture);
    vao_destroy(self->vao);
    vbo_destroy(self->vbo);
    vbo_destroy(self->ibo);
}

void renderer_shader_use(struct Renderer *self, enum ShaderType shader) {
    self->shader = self->shaders[shader];
    shader_use(self->shader);

    self->current_shader = shader;
}

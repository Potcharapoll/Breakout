#ifndef RENDERER_H
#define RENDERER_H
#include "vao.h"
#include "vbo.h"
#include "shader.h"
#include "texture.h"

#include <cglm/cglm.h>

enum RectangleColor {
    BLACK,
    WHITE,
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    PURPLE
};
#define COLOR_LAST PURPLE+1


enum ShaderType {
    SHADER_BASIC,
    SHADER_TEXT
};
#define SHADER_LAST SHADER_TEXT+1

struct Renderer {
    struct VAO vao;
    struct VBO vbo, ibo;
    mat4 proj;

    enum ShaderType current_shader;
    struct Shader shaders[SHADER_LAST];
    struct Shader shader;

    struct Texture texture;
};

void renderer_init(struct Renderer *self);
void renderer_destroy(struct Renderer *self);
void renderer_shader_use(struct Renderer *self, enum ShaderType shader);
#endif

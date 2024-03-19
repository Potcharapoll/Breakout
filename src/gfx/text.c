#include "text.h"
#include "gfx.h"
#include "texture.h"
#include "vbo.h"

#include <ctype.h>

struct Text text_init(ivec2 size, f32 frame_size) {
    struct Text text = {
        .size = {size[0], size[1]},
        .frame_size = frame_size
    };
    return text;
}

void text_render(struct Text self, struct Renderer *renderer, char *text, f32 x, f32 y, f32 scale, vec3 txt_color) {

    float size = self.frame_size * scale;
    short rows = 0;
    short columns = 0;

    const short first_alphabet = 4;
    const short first_digit    = 1;
    const short txt_padding    = 12;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    texture_bind(renderer->texture);
    renderer_shader_use(renderer, SHADER_TEXT);
    glUniform3f(glGetUniformLocation(renderer->shader.handle, "txt_color"), txt_color[0], txt_color[1], txt_color[2]);

    for (char *c = text; *c; c++) {

        if (isspace(*c)) {
            x += size - size/txt_padding;
            c++;
        }

        if (isalpha(*c)) {
            rows = *c - 'A';
            columns = first_alphabet;
        }
        else if (isdigit(*c)) {
            rows = *c - '0' + 2;
            columns = first_digit;

        }

        if (rows > 7) {
            columns -= (rows/self.frame_size);
            rows %= self.frame_size;
        }

        float vertices[] = {
            x,      y,      0.0f,     0.0f, 0.0f,
            x+size, y,      0.0f,     1.0f, 0.0f,
            x+size, y+size, 0.0f,     1.0f, 1.0f,
            x,      y+size, 0.0f,     0.0f, 1.0f
        };

        vbo_bind(renderer->vbo);
        vbo_subdata(renderer->vbo, sizeof(vertices), vertices);
        vao_attr(0, 3, GL_FLOAT, 5*sizeof(float), 0);
        vao_attr(1, 2, GL_FLOAT, 5*sizeof(float), 3*sizeof(float));

        glUniform2f(glGetUniformLocation(renderer->shader.handle, "img_size"), (float)self.size[0], (float)self.size[1]);
        glUniform2f(glGetUniformLocation(renderer->shader.handle, "uv"), (float)rows, (float)columns);
        glUniform1f(glGetUniformLocation(renderer->shader.handle, "frame_size"), (float)self.frame_size);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        
        x += size - size/txt_padding;
    }
    texture_unbind();
}



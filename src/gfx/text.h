#ifndef TEXT_H
#define TEXT_H
#include "../util/types.h"
#include "renderer.h"

#include <cglm/cglm.h>

struct Text {
    ivec2 size;
    u32 frame_size;
};

struct Text text_init(ivec2 size, f32 frame_size);
void text_render(struct Text self, struct Renderer *renderer, char *text, f32 x, f32 y, f32 scale, vec3 txt_color);
#endif

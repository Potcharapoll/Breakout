#version 330 core
out vec4 frag_color;

in vec2 tex_coord;

uniform sampler2D tex;

uniform vec3 txt_color;
uniform vec2 img_size;
uniform vec2 uv;

uniform float frame_size;

void main() {
    float x = frame_size / img_size.x;
    float y = frame_size / img_size.y;
    frag_color = vec4(txt_color, texture(tex, vec2(tex_coord.x*x, tex_coord.y*y) + vec2(x*uv.x, y*uv.y)).r);
}

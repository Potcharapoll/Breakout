#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex_pos;

out vec2 tex_coord;

uniform mat4 proj;

void main() {
    gl_Position = proj * vec4(position, 1.0);
    tex_coord = tex_pos;
}

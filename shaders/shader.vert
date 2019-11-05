#version 450 core
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 _texture_coord;
out vec2 texture_coord;
void main() {
    gl_Position = vec4(vertex_position, 1.f);
    texture_coord = _texture_coord;
}
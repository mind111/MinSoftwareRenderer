#version 450 core
in vec2 texture_coord;
out vec4 fragment_color;
uniform sampler2D bitmap_texture;
void main() {
    fragment_color = texture(bitmap_texture, texture_coord);
    //fragment_color = vec4(1.f, .2f, .3f, .1f);
}
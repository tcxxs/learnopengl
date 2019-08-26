#version 460 core

in vec3 fg_pos;
uniform samplerCube cube;
out vec4 color_out;

void main() {
    color_out = vec4(texture(cube, fg_pos).rgb, 1.0);
} 
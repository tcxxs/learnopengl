#version 460 core

#define MATH_PI 3.14159265359

in VertexAttrs {
    vec3 pos;
}vertex;

uniform samplerCube cube;

out vec4 color_out;

void main() {
    color_out = vec4(texture(cube, vertex.pos).rgb, 1.0);
}
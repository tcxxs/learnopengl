#version 460 core

#define MATH_PI 3.14159265359

in VertexAttrs {
    vec3 pos;
}vertex;

uniform samplerCube cube;
uniform float roughness;

out vec4 color_out;

void main() {
    color_out = vec4(roughness, 1.0, 0.0, 1.0);
}
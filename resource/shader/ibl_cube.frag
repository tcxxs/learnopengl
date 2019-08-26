#version 460 core

#define MATH_PI 3.14159265359

in VertexAttrs {
    vec3 pos;
}vertex;

uniform sampler2D frame;

out vec4 color_out;

void main() {
    vec3 pos = normalize(vertex.pos);
    float u = atan(pos.z, pos.x) / (2 * MATH_PI) + 0.5;
    float v = asin(pos.y) / MATH_PI + 0.5;

    vec3 color = texture(frame, vec2(u, v)).rgb;
    color_out = vec4(color, 1.0);
}
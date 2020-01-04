#version 460 core

in vec2 fg_uv;

uniform sampler2D frame;
uniform int coc_radius;

out vec4 color_out;

const int disc_num = 22;
const vec2 disc_samplers[22] = vec2[](						
    vec2(0, 0),
    vec2(0.53333336, 0),
    vec2(0.3325279, 0.4169768),
    vec2(-0.11867785, 0.5199616),
    vec2(-0.48051673, 0.2314047),
    vec2(-0.48051673, -0.23140468),
    vec2(-0.11867763, -0.51996166),
    vec2(0.33252785, -0.4169769),
    vec2(1, 0),
    vec2(0.90096885, 0.43388376),
    vec2(0.6234898, 0.7818315),
    vec2(0.22252098, 0.9749279),
    vec2(-0.22252095, 0.9749279),
    vec2(-0.62349, 0.7818314),
    vec2(-0.90096885, 0.43388382),
    vec2(-1, 0),
    vec2(-0.90096885, -0.43388376),
    vec2(-0.6234896, -0.7818316),
    vec2(-0.22252055, -0.974928),
    vec2(0.2225215, -0.9749278),
    vec2(0.6234897, -0.7818316),
    vec2(0.90096885, -0.43388376)
);

void main() {
    vec2 size = textureSize(frame, 0);
    vec2 texel = 1.0 / size;

    vec4 color = vec4(0.0);
    for (int i = 0; i < disc_num; i++) {
        vec2 offset = disc_samplers[i] * texel * coc_radius;
        color += texture(frame, fg_uv + offset);
    }
    color *= 1.0 / disc_num;

    color_out = color;
}

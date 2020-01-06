#version 460 core

#define BLOOM_LIMIT 1.0

in vec2 fg_uv;

uniform sampler2D frame;

out vec4 color_out;

void main() {
    vec4 color = texture(frame, fg_uv);
    float bright = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (bright > BLOOM_LIMIT)
        color_out = color;
    else
        color_out = vec4(0.0);
}

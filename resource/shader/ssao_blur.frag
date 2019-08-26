#version 460 core

in vec2 fg_uv;

uniform int noise;
uniform sampler2D frame;

out float color_out;

void main() {
    int noise_size = min(8, noise / 2);
    vec2 step_uv = 1.0 / vec2(textureSize(frame, 0));
    float color_total = 0.0;
    for (int x = -noise_size; x < noise_size; ++x) {
        for (int y = -noise_size; y < noise_size; ++y) {
            vec2 offset = vec2(float(x), float(y)) * step_uv;
            color_total += texture(frame, fg_uv + offset).r;
        }
    }

    color_out = color_total / float(noise * noise);
}

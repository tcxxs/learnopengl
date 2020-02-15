#version 460 core

in vec2 fg_uv;

uniform sampler2D frame;
uniform bool horizontal;

out vec4 color_out;

const float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 offset = 1.0 / textureSize(frame, 0);
    vec3 color_total = texture(frame, fg_uv).rgb * weight[0];
    if(horizontal) {
        for(int i = 1; i < 5; ++i) {
            color_total += texture(frame, fg_uv + vec2(offset.x * i, 0.0)).rgb * weight[i];
            color_total += texture(frame, fg_uv - vec2(offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else {
        for(int i = 1; i < 5; ++i) {
            color_total += texture(frame, fg_uv + vec2(0.0, offset.y * i)).rgb * weight[i];
            color_total += texture(frame, fg_uv - vec2(0.0, offset.y * i)).rgb * weight[i];
        }
    }

    color_out = vec4(color_total, 1.0);
}

#version 460 core

in vec2 fg_uv;

uniform sampler2D frame;

out vec4 color_out;

void main() {
    vec2 size = textureSize(frame, 0);
    vec2 texel = 1.0 / size;
    vec4 offset = texel.xyxy * vec2(-0.5, 0.5).xxyy;
    vec4 color = texture(frame, fg_uv + offset.xy) + texture(frame, fg_uv + offset.zy) + texture(frame, fg_uv + offset.xw) + texture(frame, fg_uv + offset.zw);
    color_out = color / 4.0;
}

#version 460 core

#define VIEW_NEAR 0.1
#define VIEW_FAR 100.0

in vec2 fg_uv;

uniform sampler2D frame;
uniform float focus_dis;
uniform float focus_range;

out float color_out;

void main() {
    vec4 ds = texture(frame, fg_uv);

    float z = ds.x * 2.0 - 1.0;
    z = (2.0 * VIEW_NEAR * VIEW_FAR) / (VIEW_FAR + VIEW_NEAR - z * (VIEW_FAR - VIEW_NEAR));

    float coc = (z - focus_dis) / focus_range;
    coc = clamp(coc, -1, 1);
    color_out = coc;
}

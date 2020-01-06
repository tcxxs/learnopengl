#version 460 core

in vec2 fg_uv;

uniform sampler2D scene;
uniform sampler2D dof;

out vec4 color_out;

void main() {
    vec4 s = texture(scene, fg_uv);
    vec4 d = texture(dof, fg_uv);
    s = mix(s, d, d.a);

    color_out = vec4(s.rgb, 1);
}

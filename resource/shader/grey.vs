#version 460 core

in vec2 vt_pos;
in vec2 vt_uv;

out vec2 fg_uv;

void main()
{
    gl_Position = vec4(vt_pos.x, vt_pos.y, 0.0, 1.0);
    fg_uv = vt_uv;
}
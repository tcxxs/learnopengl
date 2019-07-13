#version 460 core

in vec3 pos;
in vec2 uv;

out vec2 fg_uv;

void main()
{
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    fg_uv = uv;
}
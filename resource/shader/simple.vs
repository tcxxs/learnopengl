#version 460 core
in vec3 pos;
in vec3 vt_color;
in vec2 vt_uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 fg_color;
out vec2 fg_uv;

void main()
{
    gl_Position = proj * view * model * vec4(pos.x, pos.y, pos.z, 1.0);
    fg_color = vt_color;
    fg_uv = vt_uv;
}
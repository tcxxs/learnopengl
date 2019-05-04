#version 460 core
in vec3 vt_pos;
in vec3 vt_color;
in vec2 vt_uv;
in vec3 vt_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 fg_pos;
out vec3 fg_color;
out vec2 fg_uv;
out vec3 fg_normal;

void main()
{
    gl_Position = proj * view * model * vec4(vt_pos.x, vt_pos.y, vt_pos.z, 1.0);
    fg_pos = vec3(model * vec4(vt_pos, 1.0));
    fg_color = vt_color;
    fg_uv = vt_uv;
    fg_normal =vt_normal;
}
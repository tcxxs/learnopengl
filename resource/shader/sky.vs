#version 460 core
in vec3 vt_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 fg_pos;

void main()
{
    gl_Position = (proj * mat4(mat3(view)) * vec4(vt_pos, 1.0)).xyww;
    fg_pos = vt_pos;
}
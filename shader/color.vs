#version 460 core
in vec3 vt_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * model * vec4(vt_pos.x, vt_pos.y, vt_pos.z, 1.0);
}
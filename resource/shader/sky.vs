#version 460 core
in vec3 vt_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 fg_pos;

void main()
{
    gl_Position = proj * view * model * vec4(vt_pos.x, vt_pos.y, vt_pos.z, 1.0);
    fg_pos = vec3(model * vec4(vt_pos, 1.0));
}
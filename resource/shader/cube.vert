#version 460 core

in vec3 pos;

uniform MatrixVP {
    mat4 view;
    mat4 proj;
};
uniform mat4 model;

out vec3 fg_pos;

void main()
{
    fg_pos = pos;
    gl_Position = proj * view * model * vec4(pos, 1.0);
}
#version 460 core

in vec3 pos;

uniform MatrixVP {
    mat4 view;
    mat4 proj;
};
uniform mat4 model;

void main()
{
    gl_Position = proj * view * model * vec4(pos.xyz, 1.0);
}
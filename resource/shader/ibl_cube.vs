#version 460 core

in vec3 pos;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(pos.xyz, 1.0);
}
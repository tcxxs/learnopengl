#version 460 core
in vec3 pos;
uniform vec4 color;

void main()
{
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}
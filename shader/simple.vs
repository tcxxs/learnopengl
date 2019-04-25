#version 460 core
in vec3 pos;
in vec3 color;
in vec2 uv;

out vec3 fcolor;
out vec2 fuv;

void main()
{
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
    fcolor = color;
    fuv = uv;
}
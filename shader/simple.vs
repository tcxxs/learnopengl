#version 460 core
in vec3 pos;
in vec3 color;
in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 fcolor;
out vec2 fuv;

void main()
{
    gl_Position = proj * view * model * vec4(pos.x, pos.y, pos.z, 1.0);
    fcolor = color;
    fuv = uv;
}
#version 460 core

in vec3 fg_pos;

uniform samplerCube uf_cube;

out vec4 FragColor;

void main()
{
    FragColor = texture(uf_cube, fg_pos);
} 
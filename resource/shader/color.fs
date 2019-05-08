#version 460 core

uniform vec3 uf_color;

out vec4 FragColor;

void main()
{
    FragColor = vec4(uf_color, 1.0);
} 
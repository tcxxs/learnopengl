#version 460 core
in vec3 fcolor;
in vec2 fuv;

uniform sampler2D tex;

out vec4 FragColor;

void main()
{
    FragColor = texture(tex, fuv) * vec4(fcolor, 1.0);
} 
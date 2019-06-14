#version 460 core

in vec2 fg_uv;
uniform sampler2D draw;

out vec4 FragColor;

void main()
{
    vec3 color = 1.0 - texture(draw, fg_uv).rgb;
    FragColor = vec4(color, 1.0);
} 
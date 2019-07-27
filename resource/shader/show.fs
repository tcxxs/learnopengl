#version 460 core

in vec2 fg_uv;
uniform sampler2D frame;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(frame, fg_uv).rgb, 1.0);
}
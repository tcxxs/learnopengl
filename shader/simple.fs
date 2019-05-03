#version 460 core
in vec3 fcolor;
in vec2 fuv;

uniform sampler2D tex;
uniform vec3 lcolor;

out vec4 FragColor;

void main()
{
    //FragColor = texture(tex, fuv);
    FragColor = vec4(lcolor * fcolor, 1.0);
} 
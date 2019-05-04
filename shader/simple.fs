#version 460 core
in vec3 fg_color;
in vec2 fg_uv;

uniform sampler2D tex;
uniform vec3 lcolor;

out vec4 FragColor;

void main()
{
    //FragColor = texture(tex, fg_uv);
    FragColor = vec4(lcolor * fg_color, 1.0);
} 
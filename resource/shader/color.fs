#version 460 core

struct Material {
    sampler2D diffuse;
};

in vec3 fg_pos;
in vec2 fg_uv;
in vec3 fg_normal;

uniform vec3 uf_color;
uniform Material material;

out vec4 FragColor;

void main()
{
    vec3 diffuse_color = texture(material.diffuse, fg_uv).rgb;
    FragColor = vec4(diffuse_color * uf_color, 1.0);
} 
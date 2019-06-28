#version 460 core

struct Material {
    sampler2D diffuse;
};

uniform vec3 uf_color;
uniform Material material;

in VertexAttrs {
    vec3 pos;
    vec2 uv;
    vec3 normal;
}vertex;
out vec4 FragColor;

void main()
{
    vec3 diffuse_color = texture(material.diffuse, vertex.uv).rgb;
    FragColor = vec4(diffuse_color * uf_color, 1.0);
} 
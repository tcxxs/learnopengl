#version 460 core

struct Material {
    vec3 color;
    sampler2D diffuse;
};

uniform Material material;

in VertexAttrs {
    vec2 uv;
}vertex;
out vec4 color_out;

void main()
{
    vec4 diffuse_color = texture(material.diffuse, vertex.uv);
    color_out = diffuse_color * vec4(material.color, 1.0);
} 
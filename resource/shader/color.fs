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
    vec3 diffuse_color = texture(material.diffuse, vertex.uv).rgb;
    color_out = vec4(diffuse_color * material.color, 1.0);
} 
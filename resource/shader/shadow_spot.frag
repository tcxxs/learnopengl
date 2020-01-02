#version 460 core

struct Material {
    sampler2D diffuse;
};

in VertexAttrs {
    vec2 uv;
}vertex;

uniform Material material;
uniform float cutout_alpha;

void main()
{
    float alpha = texture(material.diffuse, vertex.uv).a;
    if (alpha < cutout_alpha)
        discard;
} 
#version 460 core

#define VIEW_FAR 100.0

struct Material {
    sampler2D diffuse;
};

in VertexAttrs {
    vec3 pos;
    vec2 uv;
}vertex;

uniform Material material;
uniform float cutout_alpha;
uniform vec3 light_pos;

void main()
{
    float alpha = texture(material.diffuse, vertex.uv).a;
    if (alpha < cutout_alpha)
        discard;

    float dis = length(vertex.pos - light_pos);
    dis = dis / VIEW_FAR;
    gl_FragDepth = dis;
}
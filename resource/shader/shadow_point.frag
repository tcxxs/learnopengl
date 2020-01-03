#version 460 core

#define VIEW_FAR 100.0
#define DITHER_MIN 0.5
#define DITHER_MAX 0.9
#define DITHER_PTN 16.0

struct Material {
    sampler2D diffuse;
};

in VertexAttrs {
    vec3 pos;
    vec2 uv;
}vertex;

uniform Material material;
uniform float cutout_alpha;
uniform sampler2D dither;
uniform vec3 light_pos;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    float alpha = texture(material.diffuse, vertex.uv).a;
    if (alpha < cutout_alpha)
        discard;
    else if (alpha < DITHER_MAX) {
        float u = (floor(alpha * DITHER_PTN) + rand(gl_FragCoord.xy)) / DITHER_PTN;
        float v = rand(vertex.uv);
        float d = texture(dither, vec2(u, v)).r;
        if (d > DITHER_MIN)
            discard;
    }
    
    float dis = length(vertex.pos - light_pos);
    dis = dis / VIEW_FAR;
    gl_FragDepth = dis;
}
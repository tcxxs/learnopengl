#version 460 core

#define BLOOM_LIMIT 1.0

in VertexAttrs {
    vec3 pos;
}vertex;
uniform samplerCube uf_cube;
out vec4 color_out;
out vec4 color_bloom;

void main()
{
    color_out = vec4(texture(uf_cube, vertex.pos).rgb, 1.0);
    float bright = dot(color_out.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (bright > BLOOM_LIMIT)
        color_bloom = color_out;
    else
        color_bloom = vec4(0.0);
} 
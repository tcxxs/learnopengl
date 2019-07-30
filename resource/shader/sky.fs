#version 460 core

uniform samplerCube uf_cube;

in VertexAttrs {
    vec3 pos;
}vertex;
out vec4 color_out;

void main()
{
    color_out = texture(uf_cube, vertex.pos);
} 
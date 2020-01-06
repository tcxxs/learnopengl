#version 460 core

in VertexAttrs {
    vec3 pos;
}vertex;
uniform samplerCube uf_cube;
out vec4 color_out;

void main()
{
    color_out = vec4(texture(uf_cube, vertex.pos).rgb, 1.0);
} 
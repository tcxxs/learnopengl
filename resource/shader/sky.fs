#version 460 core

uniform samplerCube uf_cube;

in VertexAttrs {
    vec3 pos;
}vertex;
out vec4 FragColor;

void main()
{
    FragColor = texture(uf_cube, vertex.pos);
} 
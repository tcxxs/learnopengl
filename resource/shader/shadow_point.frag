#version 460 core

#define VIEW_FAR 100.0

in VertexAttrs {
    vec3 pos;
}vertex;

uniform vec3 light_pos;

void main()
{
    float dis = length(vertex.pos - light_pos);
    dis = dis / VIEW_FAR;
    gl_FragDepth = dis;
}
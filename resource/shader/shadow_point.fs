#version 460 core

#define VIEW_FAR 100.0

in VertexAttrs {
    vec3 pos;
}vertex_in;

uniform vec3 light_pos;

void main()
{
    float dis = length(vertex_in.pos - light_pos);
    dis = dis / 100.0;
    gl_FragDepth = dis;
}
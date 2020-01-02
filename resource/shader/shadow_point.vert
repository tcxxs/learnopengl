#version 460 core

in vec3 pos;
in vec2 uv;

uniform mat4 model;

out VertexAttrs {
    vec2 uv;
}vertex;

void main()
{
    vertex.uv = uv;
    gl_Position = model * vec4(pos.xyz, 1.0);
}
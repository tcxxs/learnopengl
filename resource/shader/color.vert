#version 460 core

in vec3 pos;
in vec2 uv;

uniform MatrixVP {
    mat4 view;
    mat4 proj;
};
uniform mat4 model;

out VertexAttrs {
    vec2 uv;
}vertex;

void main()
{
    gl_Position = proj * view * model * vec4(pos.x, pos.y, pos.z, 1.0);
    vertex.uv = uv;
}
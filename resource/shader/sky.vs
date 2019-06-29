#version 460 core

uniform MatrixVP {
    mat4 view;
    mat4 proj;
};

in vec3 pos;
out VertexAttrs {
    vec3 pos;
}vertex;

void main()
{
    gl_Position = (proj * mat4(mat3(view)) * vec4(pos, 1.0)).xyww;
    vertex.pos = pos;
}
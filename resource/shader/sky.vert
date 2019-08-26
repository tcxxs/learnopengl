#version 460 core

in vec3 pos;

uniform MatrixVP {
    mat4 view;
    mat4 proj;
};

out VertexAttrs {
    vec3 pos;
}vertex;

void main()
{
    gl_Position = (proj * mat4(mat3(view)) * vec4(pos, 1.0)).xyww;
    vertex.pos = pos;
}
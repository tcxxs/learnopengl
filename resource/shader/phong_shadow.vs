#version 460 core

in vec3 pos;
in vec2 uv;
in vec3 normal;

uniform MatrixVP {
    mat4 view;
    mat4 proj;
};
uniform mat4 model;
uniform mat4 shadow_vp;

out VertexAttrs {
    vec3 pos;
    vec2 uv;
    vec3 normal;
}vertex;
out vec4 shadow_scpos;

void main()
{
    gl_Position = proj * view * model * vec4(pos, 1.0);
    vertex.pos = vec3(model * vec4(pos, 1.0));
    vertex.uv = uv;
    vertex.normal = mat3(transpose(inverse(model))) * normal;
    shadow_scpos = shadow_vp * vec4(vertex.pos, 1.0);
}
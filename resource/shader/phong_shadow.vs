#version 460 core

#define LIGHT_DIR 1
#define LIGHT_POINT 2
#define LIGHT_SPOT 3

in vec3 pos;
in vec2 uv;
in vec3 normal;

uniform MatrixVP {
    mat4 view;
    mat4 proj;
};
uniform mat4 model;
uniform int shadow_type;
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
    if (shadow_type == LIGHT_SPOT)
        shadow_scpos = shadow_vp * vec4(vertex.pos, 1.0);
}
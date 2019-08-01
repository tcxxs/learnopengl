#version 460 core

in vec3 pos;
in vec2 uv;
in vec3 normal;
in vec3 tangent;

uniform MatrixVP {
    mat4 view;
    mat4 proj;
};
uniform mat4 model;
uniform Scene {
    vec3 camera;
    int lights;
}scene;

out VertexAttrs {
    vec3 pos;
    vec2 uv;
    vec3 normal;
}vertex;
out TangentAttrs {
    vec3 pos;
    vec3 camera;
    mat3 tbn;
}tgvert;

void main()
{
    gl_Position = proj * view * model * vec4(pos, 1.0);

    mat3 matnor = mat3(transpose(inverse(model)));
    vertex.pos = vec3(model * vec4(pos, 1.0));
    vertex.uv = uv;
    vertex.normal = normalize(matnor * normal);

    vec3 n = normalize(vertex.normal);
    vec3 t = normalize(matnor * tangent);
    t = normalize(t - dot(t, n) * n);
    vec3 b = cross(vertex.normal, t);
    tgvert.tbn = mat3(t, b, n);

    mat3 tbni = transpose(tgvert.tbn);
    tgvert.pos = tbni * vertex.pos;
    tgvert.camera = tbni * scene.camera;
}
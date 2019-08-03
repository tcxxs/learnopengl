#version 460 core

#define SPACE_VIEW 1

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
#if SPACE_VIEW
    flat vec3 camera;
#endif
}vertex;
out TangentAttrs {
    vec3 pos;
    vec3 camera;
    mat3 tbn;
}tgvert;

void main()
{
    gl_Position = proj * view * model * vec4(pos, 1.0);

    mat4 space;
    vec3 camera;
#if SPACE_VIEW
    space = view * model;
    camera = vec3(view * vec4(scene.camera, 1.0));
#else
    space = model;
    camera = scene.camera;
#endif
    mat3 matnor = mat3(transpose(inverse(space)));
    vertex.pos = vec3(space * vec4(pos, 1.0));
    vertex.uv = uv;
    vertex.normal = normalize(matnor * normal);
#if SPACE_VIEW
    vertex.camera = camera;
#endif

    vec3 n = normalize(vertex.normal);
    vec3 t = normalize(matnor * tangent);
    t = normalize(t - dot(t, n) * n);
    vec3 b = cross(vertex.normal, t);
    tgvert.tbn = mat3(t, b, n);

    mat3 tbni = transpose(tgvert.tbn);
    tgvert.pos = tbni * vertex.pos;
    tgvert.camera = tbni * camera;
}
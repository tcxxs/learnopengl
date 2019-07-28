#version 460 core

#define LIGHT_MAX 10
#define LIGHT_DIR 1
#define LIGHT_POINT 2
#define LIGHT_SPOT 3

in vec3 pos;
in vec2 uv;
in vec3 normal;
in vec3 tangent;

uniform MatrixVP {
    mat4 view;
    mat4 proj;
};
uniform mat4 model;
uniform int shadow_probe;
uniform mat4 shadow_vp;

struct Light {
    int type;
	vec3 pos;
	vec3 dir;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    float inner;
    float outter;
};

uniform Scene {
    vec3 camera;
    int lights;
}scene;
uniform Lights {
    Light light;
}lights[LIGHT_MAX];

out VertexAttrs {
    vec3 pos;
    vec2 uv;
    vec3 normal;
}vertex;
out TangentAttrs {
    vec3 pos;
    vec3 camera;
    vec3 ltpos[LIGHT_MAX];
    vec3 ltdir[LIGHT_MAX];
}tgvert;
out vec4 shadow_scpos;

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
    mat3 tbni = transpose(mat3(t, b, n));

    tgvert.pos = tbni * vertex.pos;
    tgvert.camera = tbni * scene.camera;
    for (int i = 0; i < scene.lights; ++i) {
        tgvert.ltpos[i] = tbni * lights[i].light.pos;
        tgvert.ltdir[i] = tbni * lights[i].light.dir;
    }
    
    if (lights[shadow_probe].light.type == LIGHT_SPOT) {
        shadow_scpos = shadow_vp * vec4(vertex.pos, 1.0);
    }
}
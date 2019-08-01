#version 460 core

#define GAMMA_CORRCT 1
#define GAMMA_VAL 2.2

struct Material {
    sampler2D diffuse;
    float specular_factor;
    sampler2D specular;
    sampler2D normal;
    float displace_factor;
    sampler2D displace;
};

struct TextureArg {
    bool diffuse;
    bool specular;
    bool normal;
    bool displace;
};

struct ColorArg {
    vec3 diffuse;
    vec3 specular;
};

struct CalcArg {
    vec3 pos;
    vec2 uv;
    vec3 normal;
    vec3 camera;
    vec3 camdir;
    TextureArg tex;
    ColorArg color;
}calc;

in VertexAttrs {
    vec3 pos;
    vec2 uv;
    vec3 normal;
}vertex;
in TangentAttrs {
    vec3 pos;
    vec3 camera;
    mat3 tbn;
}tgvert;

uniform Material material;
uniform Scene {
    vec3 camera;
    int lights;
}scene;

out vec4 color_pos;
out vec4 color_normal;
out vec4 color_albedo;

void init_calc() {
    calc.pos = vertex.pos;
    calc.uv = vertex.uv;
    calc.normal = vertex.normal;
    calc.camera = scene.camera;
    calc.camdir = normalize(calc.camera - calc.pos);

    calc.tex.diffuse = false;
    if (textureSize(material.diffuse, 0).x > 1) {
        calc.tex.diffuse = true;
    }
    calc.tex.specular = false;
    if (textureSize(material.specular, 0).x > 1) {
        calc.tex.specular = true;
    }
    calc.tex.normal = false;
    if (textureSize(material.normal, 0).x > 1) {
        calc.tex.normal = true;
    }
    calc.tex.displace = false;
    if (textureSize(material.displace, 0).x > 1) {
        calc.tex.displace = true;
    }
}

void calc_displace() {
    if (!calc.tex.normal || !calc.tex.displace)
        return;

    vec3 camdir = normalize(tgvert.camera - tgvert.pos);

    const float layer_min = 8;
    const float layer_max = 32;
    float layers = mix(layer_max, layer_min, abs(dot(vec3(0.0, 0.0, 1.0), camdir)));
    float delta_depth = 1.0 / layers;
    vec2 view = camdir.xy * material.displace_factor; 
    vec2 delta_uv = view / layers;

    float check_depth = 0.0;
    vec2  cur_uv = vertex.uv;
    float cur_depth = texture(material.displace, cur_uv).r;
    float prev_depth = cur_depth;
    while(check_depth < cur_depth) {
        prev_depth = cur_depth;
        cur_uv -= delta_uv;
        cur_depth = texture(material.displace, cur_uv).r;  
        check_depth += delta_depth;  
    }

    float after  = cur_depth - check_depth;
    float before = prev_depth - check_depth + delta_depth;
    float weight = after / (after - before);
    calc.uv = (cur_uv + delta_uv) * weight + cur_uv * (1.0 - weight);
}

void sample_texture() {
    calc.color.diffuse = vec3(1.0);
    if (calc.tex.diffuse) {
        calc.color.diffuse = texture(material.diffuse, calc.uv).rgb;
    }
    #if GAMMA_CORRCT
    calc.color.diffuse = pow(calc.color.diffuse, vec3(GAMMA_VAL));
    #endif

    calc.color.specular = vec3(0.0);
    if (calc.tex.specular) {
        calc.color.specular = texture(material.specular, calc.uv).rgb;
    }

    if (calc.tex.normal) {
        calc.normal = texture(material.normal, calc.uv).rgb;
        calc.normal = normalize(calc.normal * 2.0 - 1.0);
        calc.normal = normalize(tgvert.tbn * calc.normal);
    }
}

void main() {
    init_calc();
    calc_displace();
    sample_texture();

    color_pos = vec4(calc.pos, gl_FragCoord.z);
    color_normal = vec4(calc.normal, 1.0);
    color_albedo = vec4(calc.color.diffuse, calc.color.specular.r);
} 
#version 460 core

#define SPACE_VIEW 1
#define VIEW_NEAR 0.1
#define VIEW_FAR 100.0

#define GAMMA_CORRCT 1
#define GAMMA_VAL 2.2

struct Material {
    sampler2D diffuse;
    sampler2D normal;
    sampler2D displace;
    float displace_factor;
    sampler2D metallic;
    sampler2D roughness;
    sampler2D ao;
    sampler2D mask;
    float mask_factor;
};

struct TextureArg {
    bool diffuse;
    bool normal;
    bool displace;
    bool metallic;
    bool roughness;
    bool ao;
    bool mask;
};

struct ColorArg {
    vec3 diffuse;
    float metallic;
    float roughness;
    float ao;
};

struct CalcArg {
    vec3 pos;
    float depth;
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
#if SPACE_VIEW
    flat vec3 camera;
#endif
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
out vec4 color_pbr;

void init_calc() {
    calc.pos = vertex.pos;
    calc.uv = vertex.uv;
    calc.normal = vertex.normal;
#if SPACE_VIEW
    calc.camera = vertex.camera;
#else
    calc.camera = scene.camera;
#endif
    calc.camdir = normalize(calc.camera - calc.pos);

    calc.tex.diffuse = false;
    if (textureSize(material.diffuse, 0).x > 1) {
        calc.tex.diffuse = true;
    }
    calc.tex.normal = false;
    if (textureSize(material.normal, 0).x > 1) {
        calc.tex.normal = true;
    }
    calc.tex.displace = false;
    if (textureSize(material.displace, 0).x > 1) {
        calc.tex.displace = true;
    }
    calc.tex.metallic = false;
    if (textureSize(material.metallic, 0).x > 1) {
        calc.tex.metallic = true;
    }
    calc.tex.roughness = false;
    if (textureSize(material.roughness, 0).x > 1) {
        calc.tex.roughness = true;
    }
    calc.tex.ao = false;
    if (textureSize(material.ao, 0).x > 1) {
        calc.tex.ao = true;
    }
    calc.tex.mask = false;
    if (textureSize(material.mask, 0).x > 1) {
        calc.tex.mask = true;
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
    if (calc.tex.mask) {
        float m = texture(material.mask, calc.uv).r;
        if (m < material.mask_factor)
            discard;
    }

    calc.color.diffuse = vec3(1.0);
    if (calc.tex.diffuse) {
        calc.color.diffuse = texture(material.diffuse, calc.uv).rgb;
    }
    #if GAMMA_CORRCT
    calc.color.diffuse = pow(calc.color.diffuse, vec3(GAMMA_VAL));
    #endif

    calc.color.metallic = 0.0;
    if (calc.tex.metallic) {
        calc.color.metallic = texture(material.metallic, calc.uv).r;
    }
    calc.color.roughness = 0.0;
    if (calc.tex.roughness) {
        calc.color.roughness = texture(material.roughness, calc.uv).r;
    }
    calc.color.ao = 1.0;
    if (calc.tex.ao) {
        calc.color.ao = texture(material.ao, calc.uv).r;
    }
    #if GAMMA_CORRCT
    calc.color.ao = pow(calc.color.ao, GAMMA_VAL);
    #endif

    if (calc.tex.normal) {
        calc.normal = texture(material.normal, calc.uv).rgb;
        calc.normal = normalize(calc.normal * 2.0 - 1.0);
        calc.normal = normalize(tgvert.tbn * calc.normal);
    }
}

void calc_depth() {
    float z;
#if SPACE_VIEW
    z = gl_FragCoord.z * 2.0 - 1.0;
    z = (2.0 * VIEW_NEAR * VIEW_FAR) / (VIEW_FAR + VIEW_NEAR - z * (VIEW_FAR - VIEW_NEAR));
#else
    z = gl_FragCoord.z;
#endif
    calc.depth = z;
}

void main() {
    init_calc();
    calc_displace();
    sample_texture();
    calc_depth();

    color_pos = vec4(calc.pos, calc.depth);
    color_normal = vec4(calc.normal, 1.0);
    color_albedo = vec4(calc.color.diffuse, calc.color.ao);
    color_pbr = vec4(calc.color.roughness, calc.color.metallic, 0.0, 0.0);
} 
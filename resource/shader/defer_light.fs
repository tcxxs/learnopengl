#version 460 core

#define VIEW_FAR 100.0
#define GAMMA_CORRCT 1
#define GAMMA_VAL 2.2

#define LIGHT_MAX 10
#define LIGHT_DIR 1
#define LIGHT_POINT 2
#define LIGHT_SPOT 3

#define SPECULAR_FUNC blinn_specular
#define BLOOM_LIMIT 1.0

struct Material {
    float specular_factor;
};

struct GBuffer {
    sampler2D position;
    sampler2D normal;
    sampler2D albedo;
};

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

struct ColorArg {
    vec3 diffuse;
    float specular;
};

struct LightArg {
    int ind;
    vec3 pos;
    vec3 dir;
    vec3 indir;
    float factor;
    float shadow;
};

struct CalcArg {
    vec3 pos;
    vec2 uv;
    vec3 normal;
    vec3 camera;
    vec3 camdir;
    ColorArg color;
    LightArg light;
}calc;

in vec2 fg_uv;

uniform Material material;
uniform Scene {
    vec3 camera;
    int lights;
}scene;
uniform Lights {
    Light light;
}lights[LIGHT_MAX];

uniform int shadow_probe;
uniform samplerCube shadow_cube;
uniform mat4 shadow_vp;
uniform sampler2D shadow_map;

uniform GBuffer gbuffer;

out vec4 color_out;
out vec4 color_bloom;

void init_calc() {
    vec4 position = texture(gbuffer.position, fg_uv);
    vec4 normal = texture(gbuffer.normal, fg_uv);
    vec4 albedo = texture(gbuffer.albedo, fg_uv);
    gl_FragDepth = position.a;

    calc.pos = position.rgb;
    calc.normal = normal.rgb;
    calc.camera = scene.camera;
    calc.camdir = normalize(calc.camera - calc.pos);

    calc.color.diffuse = albedo.rgb;
    #if GAMMA_CORRCT
    calc.color.diffuse = pow(calc.color.diffuse, vec3(GAMMA_VAL));
    #endif

    calc.color.specular = albedo.a;
}

void calc_dir(Light light) {
    calc.light.indir = normalize(calc.light.dir);
}

void calc_point(Light light) {
    calc.light.indir = normalize(calc.pos - calc.light.pos);
    float distance = length(calc.pos - calc.light.pos);
    calc.light.factor = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
}

void calc_spot(Light light)
{
    calc.light.indir = normalize(calc.pos - calc.light.pos);
    float theta = dot(calc.light.indir, normalize(calc.light.dir));
    if(theta > light.outter) {       
        float distance = length(calc.pos - calc.light.pos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        float intensity = clamp((theta - light.outter) / (light.inner - light.outter), 0.0, 1.0);
        calc.light.factor = attenuation * intensity;
    }
    else {
        calc.light.factor = 0.0;
    }
}

void shadow_point(Light light) {
    if (textureSize(shadow_cube, 0).x <= 1) {
        calc.light.shadow = 1.0;
        return;
    }

    float bias = max(0.001 * (1.0 - dot(calc.normal, -calc.light.indir)), 0.0005);
    vec3 shadow_offset[20] = vec3[] (
    vec3(1,  1,  1), vec3(1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
    vec3(1,  1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
    vec3(1,  1,  0), vec3(1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
    vec3(1,  0,  1), vec3(-1,  0,  1), vec3(1,  0, -1), vec3(-1,  0, -1),
    vec3(0,  1,  1), vec3(0, -1,  1), vec3(0, -1, -1), vec3(0,  1, -1)
    );
    float shadow_total = 0.0;
    float shadow_step = (1.0 + (length(calc.camdir) / VIEW_FAR)) / (textureSize(shadow_cube, 0).x / 4);
    // 不用normalize，因为要算长度
    vec3 texdir = calc.pos - calc.light.pos;
    for(int i = 0; i < 20; ++i) {
        float depth = texture(shadow_cube, (texdir + shadow_offset[i] * shadow_step)).r * VIEW_FAR;
        shadow_total += length(texdir) - bias > depth ? 0.0: 1.0;
    }
    calc.light.shadow = shadow_total / 20.0;
}

void shadow_spot(Light light) {
    if (textureSize(shadow_map, 0).x <= 1) {
        calc.light.shadow = 1.0;
        return;
    }

    vec4 scpos = shadow_vp * vec4(calc.pos, 1.0);
    vec3 coords = scpos.xyz / scpos.w;
    coords = coords * 0.5 + 0.5;
    if (coords.z > 1.0) {
        calc.light.shadow = 1.0;
        return;
    }

    float bias = max(0.001 * (1.0 - dot(calc.normal, -calc.light.indir)), 0.0005);
    float shadow_total = 0.0;
    vec2 shadow_step = 1.0 / textureSize(shadow_map, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float depth = texture(shadow_map, coords.xy + vec2(x, y) * shadow_step).r; 
            shadow_total += coords.z - bias > depth  ? 0.0: 1.0;
        }
    }
    calc.light.shadow = shadow_total / 9.0;
}

vec3 phong_ambient(vec3 light_color) {
    return light_color * calc.color.diffuse;
}

vec3 phong_diffuse(vec3 light_color) {
    float fac = max(dot(-calc.light.indir, calc.normal), 0.0);
	return light_color * calc.color.diffuse * fac;
}

vec3 phong_specular(vec3 light_color) {
    vec3 reflect_dir = reflect(calc.light.indir, calc.normal);
    float fac = pow(max(dot(calc.camdir, reflect_dir), 0.0), material.specular_factor);
	return light_color * calc.color.specular * fac;
}

vec3 blinn_specular(vec3 light_color) {
    vec3 halfdir = normalize(-calc.light.indir + calc.camdir);
    float fac = pow(max(dot(halfdir, calc.normal), 0.0), material.specular_factor);
	return light_color * calc.color.specular * fac;
}

vec3 phong_calc(Light light) {
    // TODO: 这里应该用subproduce来优化
    switch (light.type) {
    case LIGHT_DIR:
        calc_dir(light);
        break;
    case LIGHT_POINT:
        calc_point(light);
        if (calc.light.ind == shadow_probe)
            shadow_point(light);
        break;
    case LIGHT_SPOT:
        calc_spot(light);
        if (calc.light.ind == shadow_probe)
            shadow_spot(light);
        break;
    }

    vec3 color = vec3(0.0);
    if (calc.light.factor <= 0)
        return color;
    color += phong_ambient(light.ambient) * calc.light.factor;
    if (calc.light.shadow <= 0)
        return color;
    color += phong_diffuse(light.diffuse) * calc.light.factor * calc.light.shadow;
    color += SPECULAR_FUNC(light.specular) * calc.light.factor * calc.light.shadow;
    return color;
}

void main() {
    init_calc();

	vec3 color_total = vec3(0.0);
    for (int i = 0; i < scene.lights; ++i) {
        calc.light.ind = i;
        calc.light.pos = lights[i].light.pos;
        calc.light.dir = lights[i].light.dir;
        calc.light.indir = vec3(0.0);
        calc.light.factor = 1.0;
        calc.light.shadow = 1.0;
        color_total += phong_calc(lights[i].light);
    }

    color_out = vec4(color_total, 1.0);
    float bright = dot(color_out.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (bright > BLOOM_LIMIT)
        color_bloom = color_out;
    else
        color_bloom = vec4(0.0);
} 
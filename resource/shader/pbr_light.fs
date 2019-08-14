#version 460 core

#define MATH_PI 3.14159265359
#define MATH_EPS 0.00001

#define SPACE_VIEW 1

#define VIEW_NEAR 0.1
#define VIEW_FAR 100.0

#define LIGHT_MAX 10
#define LIGHT_DIR 1
#define LIGHT_POINT 2
#define LIGHT_SPOT 3

#define BLOOM_LIMIT 1.0
#define PBR_BASEF0 vec3(0.04)

struct Material {
    float specular_factor;
};

struct Shadow {
    int probe;
    samplerCube point;
    mat4 spot_vp;
    sampler2D spot_map;
};

struct GBuffer {
    sampler2D position;
    sampler2D normal;
    sampler2D albedo;
    sampler2D pbr;
};

struct Light {
    int type;
	vec3 pos;
	vec3 dir;
    vec3 color;
    float inner;
    float outter;
};

struct ColorArg {
    vec3 diffuse;
    vec3 specular;
    float metallic;
    float roughness;
    float ao;
};

struct LightArg {
    int ind;
    vec3 pos;
    vec3 dir;
    vec3 indir;
    vec3 inhalf;
    float distance;
    vec3 radiance;
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

#if SPACE_VIEW
uniform MatrixVP {
    mat4 view;
    mat4 proj;
};
#endif
uniform Scene {
    vec3 camera;
    int lights;
}scene;
uniform Lights {
    Light light;
}lights[LIGHT_MAX];

uniform Material material;
uniform Shadow shadow;
uniform GBuffer gbuffer;
uniform samplerCube ibl;
uniform sampler2D ssao;

out vec4 color_out;
out vec4 color_bloom;

void init_calc() {
    vec4 position = texture(gbuffer.position, fg_uv);
    vec4 normal = texture(gbuffer.normal, fg_uv);
    vec4 albedo = texture(gbuffer.albedo, fg_uv);
    vec4 pbr = texture(gbuffer.pbr, fg_uv);

#if SPACE_VIEW
    // 这里在geo中可能是空的点，需要判断下
    if (length(position.rgb) < MATH_EPS && abs(position.a - 1.0) < MATH_EPS)
        discard;
    float z = (VIEW_FAR + VIEW_NEAR - (2.0 * VIEW_NEAR * VIEW_FAR) / position.a) / (VIEW_FAR - VIEW_NEAR);
    gl_FragDepth = (z + 1.0) / 2.0;
#else
    gl_FragDepth = position.a;
#endif

    calc.pos = position.rgb;
    calc.normal = normal.rgb;
#if SPACE_VIEW
    calc.camera = vec3(view * vec4(scene.camera, 1.0));
#else
    calc.camera = scene.camera;
#endif
    calc.camdir = normalize(calc.camera - calc.pos);

    calc.color.metallic = albedo.a;
    calc.color.roughness = pbr.r + MATH_EPS;
    calc.color.diffuse = albedo.rgb * (1 - calc.color.metallic);
    calc.color.specular = mix(PBR_BASEF0, albedo.rgb, calc.color.metallic);
    calc.color.ao = pbr.g;
    if (textureSize(ssao, 0).x > 1) {
        calc.color.ao *= texture(ssao, fg_uv).r;
    }
}

void shadow_point(Light light) {
    if (textureSize(shadow.point, 0).x <= 1) {
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
    float shadow_step = (1.0 + (length(calc.camdir) / VIEW_FAR)) / (textureSize(shadow.point, 0).x / 4);
    // 不用normalize，因为要算长度，需要世界空间
#if SPACE_VIEW
    vec3 texdir = vec3(inverse(view) * vec4(calc.pos, 1.0)) - lights[calc.light.ind].light.pos;
#else
    vec3 texdir = calc.pos - calc.light.pos;
#endif
    for(int i = 0; i < 20; ++i) {
        float depth = texture(shadow.point, (texdir + shadow_offset[i] * shadow_step)).r * VIEW_FAR;
        shadow_total += length(texdir) - bias > depth ? 0.0: 1.0;
    }
    calc.light.shadow = shadow_total / 20.0;
}

void shadow_spot(Light light) {
    if (textureSize(shadow.spot_map, 0).x <= 1) {
        calc.light.shadow = 1.0;
        return;
    }

    // 转移到spot空间
    vec4 scpos = shadow.spot_vp * inverse(view) * vec4(calc.pos, 1.0);
    vec3 coords = scpos.xyz / scpos.w;
    coords = coords * 0.5 + 0.5;
    if (coords.z > 1.0) {
        calc.light.shadow = 1.0;
        return;
    }

    float bias = max(0.001 * (1.0 - dot(calc.normal, -calc.light.indir)), 0.0005);
    float shadow_total = 0.0;
    vec2 shadow_step = 1.0 / textureSize(shadow.spot_map, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float depth = texture(shadow.spot_map, coords.xy + vec2(x, y) * shadow_step).r; 
            shadow_total += coords.z - bias > depth  ? 0.0: 1.0;
        }
    }
    calc.light.shadow = shadow_total / 9.0;
}

vec3 F_schlick() {
    float theta = max(dot(calc.camdir, calc.light.inhalf), 0.0);
    return calc.color.specular + (1.0 - calc.color.specular) * pow(1.0 - theta, 5.0);
}

vec3 F_schlick_roughness() {
    float theta = max(dot(calc.normal, calc.camdir), 0.0);
    vec3 falloff = max(vec3(1.0 - calc.color.roughness), calc.color.specular);
    return calc.color.specular + (falloff - calc.color.specular) * pow(1.0 - theta, 5.0);
}

float D_ggx() {
    float a = calc.color.roughness * calc.color.roughness;
    float a2 = a * a;
    float theta  = max(dot(calc.normal, calc.light.inhalf), 0.0);
    float theta2 = theta * theta;
    float denom = (theta2 * (a2 - 1.0) + 1.0);

    return a2 / (MATH_PI * denom * denom);
}

float G_schlick_ggx(vec3 vector) {
    float a = calc.color.roughness * calc.color.roughness;
    float k = a / 2.0;
    float theta   = max(dot(calc.normal, vector), 0.0);

    return theta / (theta * (1.0 - k) + k);
}

float G_smith() {
    float ggx1  = G_schlick_ggx(calc.light.indir);
    float ggx2  = G_schlick_ggx(calc.camdir);

    return ggx1 * ggx2;
}

vec3 light_radiance() {
    vec3 fac_f = F_schlick();
    float fac_d = D_ggx();
    float fac_g = G_smith();

    vec3 ks = fac_f;
    vec3 kd = (vec3(1.0) - ks) * (1.0 - calc.color.metallic);
    float theta_v = max(dot(calc.normal, calc.camdir), 0.0);
    float theta_l = max(dot(calc.normal, calc.light.indir), 0.0);

    vec3 diffuse = kd * calc.color.diffuse / MATH_PI;
    vec3 specular = (fac_f * fac_d * fac_g) / (4.0 * theta_v * theta_l + MATH_EPS);
    return (diffuse + specular) * calc.light.radiance * theta_l;
}

void light_arg(Light light) {
#if SPACE_VIEW
    calc.light.pos = vec3(view * vec4(light.pos, 1.0));
    calc.light.dir = normalize(vec3(view * vec4(light.dir, 1.0) - view * vec4(0.0, 0.0, 0.0, 1.0)));
#else
    calc.light.pos = light.pos;
    calc.light.dir = normalize(light.dir);
#endif
    calc.light.indir = vec3(0.0);
    calc.light.factor = 1.0;
    calc.light.shadow = 1.0;

    switch (light.type) {
    case LIGHT_DIR:
        calc.light.indir = normalize(-calc.light.dir);
        break;
    case LIGHT_POINT:
        calc.light.indir = normalize(calc.light.pos - calc.pos);
        calc.light.distance = length(calc.pos - calc.light.pos);
        calc.light.factor = 1 / (calc.light.distance * calc.light.distance);
        if (calc.light.ind == shadow.probe)
            shadow_point(light);
        break;
    case LIGHT_SPOT:
        calc.light.indir = normalize(calc.light.pos - calc.pos);
        calc.light.distance = length(calc.pos - calc.light.pos);
        float theta = dot(calc.light.indir, normalize(-calc.light.dir));
        if(theta > light.outter) {
            float intensity = clamp((theta - light.outter) / (light.inner - light.outter), 0.0, 1.0);
            calc.light.factor = intensity / (calc.light.distance * calc.light.distance);
        }
        else {
            calc.light.factor = 0.0;
        }
        if (calc.light.ind == shadow.probe)
            shadow_spot(light);
        break;
    }

    calc.light.inhalf = normalize(calc.camdir + calc.light.indir);
    calc.light.radiance = light.color * calc.light.factor * calc.light.shadow;
}

vec3 env_diffuse() {
#if SPACE_VIEW
    vec3 normal = normalize(vec3(inverse(view) * vec4(calc.normal, 1.0)));
#else
    vec3 normal = calc.normal;
#endif
    vec3 kd = 1.0 - F_schlick_roughness();
    vec3 radiance = calc.color.diffuse * texture(ibl, normal).rgb;
    return (kd * radiance) * calc.color.ao;
}

void main() {
    init_calc();

	vec3 light = vec3(0.0);
    for (int i = 0; i < scene.lights; ++i) {
        calc.light.ind = i;
        light_arg(lights[i].light);
        if (calc.light.radiance.r > 0 || calc.light.radiance.g > 0 || calc.light.radiance.b > 0)
            light += light_radiance();
    }

    vec3 env = env_diffuse();

    color_out = vec4(env + light, 1.0);
    float bright = dot(color_out.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (bright > BLOOM_LIMIT)
        color_bloom = color_out;
    else
        color_bloom = vec4(0.0);
} 
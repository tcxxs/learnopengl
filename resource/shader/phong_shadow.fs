#version 460 core

#define LIGHT_MAX 10
#define LIGHT_DIR 1
#define LIGHT_POINT 2
#define LIGHT_SPOT 3

#define VIEW_FAR 100.0

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
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

struct LightArg {
    vec3 dir;
    float factor;
};

uniform Material material;
uniform Scene {
    vec3 camera;
    int lights;
}scene;
uniform Lights {
    Light light;
}lights[LIGHT_MAX];

uniform int shadow_type;
uniform samplerCube shadow_cube;
uniform vec3 shadow_pos;
uniform sampler2D shadow_map;

in VertexAttrs {
    vec3 pos;
    vec2 uv;
    vec3 normal;
}vertex;
in vec4 shadow_scpos;
out vec4 FragColor;

#define SPECULAR_FUNC blinn_specular
#define GAMMA_CORRCT 1
#define GAMMA_VAL 2.2

vec3 phong_ambient(vec3 light_color, vec3 diffuse) {
    return light_color * diffuse;
}

vec3 phong_diffuse(vec3 light_color, vec3 light_dir, vec3 normal, vec3 diffuse) {
    float fac = max(dot(-light_dir, normal), 0.0);
	return light_color * diffuse * fac;
}

vec3 phong_specular(vec3 light_color, vec3 light_dir, vec3 normal, vec3 specular, vec3 camera_dir) {
    vec3 reflect_dir = reflect(light_dir, normal);
    float fac = pow(max(dot(camera_dir, reflect_dir), 0.0), material.shininess);
	return light_color * specular * fac;
}

vec3 blinn_specular(vec3 light_color, vec3 light_dir, vec3 normal, vec3 specular, vec3 camera_dir) {
    vec3 halfdir = normalize(-light_dir + camera_dir);
    float fac = pow(max(dot(halfdir, normal), 0.0), material.shininess);
	return light_color * specular * fac;
}

float shadow_point(vec3 normal, Light light) {
    if (textureSize(shadow_cube, 0).x <= 1) {
        return 1.0;
    }

    vec3 dir = vertex.pos - light.pos;
    float bias = max(0.001 * (1.0 - dot(normal, -dir)), 0.0005);
    vec3 shadow_offset[20] = vec3[] (
    vec3(1,  1,  1), vec3(1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
    vec3(1,  1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
    vec3(1,  1,  0), vec3(1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
    vec3(1,  0,  1), vec3(-1,  0,  1), vec3(1,  0, -1), vec3(-1,  0, -1),
    vec3(0,  1,  1), vec3(0, -1,  1), vec3(0, -1, -1), vec3(0,  1, -1)
    );
    float shadow_total = 0.0;
    float shadow_step = (1.0 + (length(vertex.pos - scene.camera) / VIEW_FAR)) / (textureSize(shadow_cube, 0).x / 4);
    for(int i = 0; i < 20; ++i) {
        float depth = texture(shadow_cube, dir + shadow_offset[i] * shadow_step).r;
        depth *= VIEW_FAR;
        shadow_total += length(dir) - bias > depth ? 0.0: 1.0;
    }
    return shadow_total / 20.0;
}

float shadow_spot(vec3 normal, Light light) {
    if (textureSize(shadow_map, 0).x <= 1) {
        return 1.0;
    }

    vec3 coords = shadow_scpos.xyz / shadow_scpos.w;
    coords = coords * 0.5 + 0.5;
    if (coords.z > 1.0)
        return 1.0;

    vec3 dir = vertex.pos - light.pos;
    float bias = max(0.001 * (1.0 - dot(normal, -dir)), 0.0005);
    float shadow_total = 0.0;
    vec2 shadow_step = 1.0 / textureSize(shadow_map, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float depth = texture(shadow_map, coords.xy + vec2(x, y) * shadow_step).r; 
            shadow_total += coords.z - bias > depth  ? 0.0: 1.0;
        }
    }
    return shadow_total / 9.0;
}

LightArg calc_dir(Light light) {
    LightArg arg;
    arg.dir = normalize(light.dir);
    return arg;
}

LightArg calc_point(Light light) {
    LightArg arg;
    arg.dir = normalize(vertex.pos - light.pos);
    float distance = length(light.pos - vertex.pos);
    arg.factor = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    return arg;
}

LightArg calc_spot(Light light)
{
    LightArg arg;
    arg.dir = normalize(vertex.pos - light.pos);
    float theta = dot(arg.dir, normalize(light.dir));
    if(theta > light.outter) {       
        float distance = length(light.pos - vertex.pos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        float intensity = clamp((theta - light.outter) / (light.inner - light.outter), 0.0, 1.0);
        arg.factor = attenuation * intensity;
    }
    else {
        arg.factor = 0.0;
    }
    return arg;
}

void main()
{
    vec3 diffuse_color = texture(material.diffuse, vertex.uv).rgb;
    #if GAMMA_CORRCT
    diffuse_color = pow(diffuse_color, vec3(GAMMA_VAL));
    #endif
    bool specular_enable = false;
    vec3 specular_color = vec3(0.0);
    if (textureSize(material.specular, 0).x > 1) {
        specular_enable = true;
        specular_color = texture(material.specular, vertex.uv).rgb;
    }
    vec3 camera_dir = normalize(scene.camera - vertex.pos);
    vec3 normal = normalize(vertex.normal);

	vec3 color_total = vec3(0.0);
    LightArg light_arg;
    float shadow_fac;
    for (int i = 0; i < scene.lights; ++i) {
        // TODO: 这里应该用subproduce来优化
        Light light = lights[i].light;
        switch (light.type) {
        case LIGHT_DIR:
            light_arg = calc_dir(light);
            shadow_fac = 1.0;
            break;
        case LIGHT_POINT:
            light_arg = calc_point(light);
            shadow_fac = shadow_point(normal, light);
            break;
        case LIGHT_SPOT:
            light_arg = calc_spot(light);
            shadow_fac = shadow_spot(normal, light);
            break;
        }

        if (light_arg.factor > 0) {
            vec3 color_per = phong_ambient(light.ambient, diffuse_color) * light_arg.factor;
            if (shadow_fac > 0) {
                color_per += phong_diffuse(light.diffuse, light_arg.dir, normal, diffuse_color) * light_arg.factor * shadow_fac;
                if (specular_enable) {
                    color_per += SPECULAR_FUNC(light.specular, light_arg.dir, normal, specular_color, camera_dir) * light_arg.factor * shadow_fac;
                }
            }
            color_total += color_per;
        }
    }

    #if GAMMA_CORRCT
    color_total = pow(color_total, vec3(1.0/GAMMA_VAL));
    #endif
    FragColor = vec4(color_total, 1.0);
} 
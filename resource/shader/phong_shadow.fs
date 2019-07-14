#version 460 core

#define LIGHT_MAX 10
#define LIGHT_DIR 1
#define LIGHT_POINT 2
#define LIGHT_SPOT 3

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

uniform Material material;
uniform Scene {
    vec3 camera;
    int lights;
}scene;
uniform Lights {
    Light light;
}lights[LIGHT_MAX];
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

vec3 phong_ambient(vec3 color, float factory, vec3 diffuse) {
    return color * diffuse * factory;
}

vec3 phong_diffuse(vec3 color, float factory, vec3 dir, vec3 normal, vec3 diffuse) {
    float fac = max(dot(-dir, normal), 0.0);
	return color * diffuse * fac * factory;
}

vec3 phong_specular(vec3 color, float factory, vec3 dir, vec3 normal, vec3 camera_dir, vec3 specular) {
    vec3 reflect_dir = reflect(dir, normal);
    float fac = pow(max(dot(camera_dir, reflect_dir), 0.0), material.shininess);
	return color * specular * fac * factory;
}

vec3 blinn_specular(vec3 color, float factory, vec3 dir, vec3 normal, vec3 camera_dir, vec3 specular) {
    vec3 halfdir = normalize(-dir + camera_dir);
    float fac = pow(max(dot(halfdir, normal), 0.0), material.shininess);
	return color * specular * fac * factory;
}

float shadow_factor() {
    vec3 coords = shadow_scpos.xyz / shadow_scpos.w;
    coords = coords * 0.5 + 0.5;
    float depth = texture(shadow_map, coords.xy).r;
    return coords.z > depth  ? 1.0 : 0.0;
}

vec3 calc_dir(Light light, vec3 camera_dir, vec3 normal, vec3 diffuse_color, vec3 specular_color, float shadow_fac) {
    vec3 dir = normalize(light.dir);

	vec3 ambient = phong_ambient(light.ambient, 1.0, diffuse_color);
    vec3 diffuse = phong_diffuse(light.diffuse, 1.0, dir, normal, diffuse_color);
    vec3 specular = SPECULAR_FUNC(light.specular, 1.0, dir, normal, camera_dir, specular_color);

    return ambient + (diffuse + specular) * shadow_fac;
}

vec3 calc_point(Light light, vec3 camera_dir, vec3 normal, vec3 diffuse_color, vec3 specular_color, float shadow_fac)
{
    vec3 dir = normalize(vertex.pos - light.pos);
    float distance = length(light.pos - vertex.pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 ambient = phong_ambient(light.ambient, attenuation, diffuse_color);
    vec3 diffuse = phong_diffuse(light.diffuse, attenuation, dir, normal, diffuse_color);
    vec3 specular = SPECULAR_FUNC(light.specular, attenuation, dir, normal, camera_dir, specular_color);

    return ambient + (diffuse + specular) * shadow_fac;
}

vec3 calc_spot(Light light, vec3 camera_dir, vec3 normal, vec3 diffuse_color, vec3 specular_color, float shadow_fac)
{
    vec3 dir = normalize(vertex.pos - light.pos);
    float theta = dot(dir, normalize(light.dir));
    if(theta > light.outter) {       
        float distance = length(light.pos - vertex.pos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        float intensity = clamp((theta - light.outter) / (light.inner - light.outter), 0.0, 1.0);   

        vec3 ambient = phong_ambient(light.ambient, attenuation * intensity, diffuse_color);
        vec3 diffuse = phong_diffuse(light.diffuse, attenuation * intensity, dir, normal, diffuse_color);
        vec3 specular = SPECULAR_FUNC(light.specular, attenuation * intensity, dir, normal, camera_dir, specular_color);

        return ambient + (diffuse + specular) * shadow_fac;
    }
    else {
       return vec3(0.0);
    }
}

void main()
{
    vec3 diffuse_color = texture(material.diffuse, vertex.uv).rgb;
    #if GAMMA_CORRCT
    diffuse_color = pow(diffuse_color, vec3(GAMMA_VAL));
    #endif
    vec3 specular_color = texture(material.specular, vertex.uv).rgb;
    float shadow_fac = shadow_factor();
    vec3 camera_dir = normalize(scene.camera - vertex.pos);
    vec3 normal = normalize(vertex.normal);

	vec3 color = vec3(0.0);
    for (int i = 0; i < scene.lights; ++i) {
        // TODO: 这里应该用subproduce来优化
        switch (lights[i].light.type) {
        case LIGHT_DIR:
            color += calc_dir(lights[i].light, camera_dir, normal, diffuse_color, specular_color, shadow_fac);
            break;
        case LIGHT_POINT:
            color += calc_point(lights[i].light, camera_dir, normal, diffuse_color, specular_color, shadow_fac);
            break;
        case LIGHT_SPOT:
            color += calc_spot(lights[i].light, camera_dir, normal, diffuse_color, specular_color, shadow_fac);
            break;
        }
    }

    #if GAMMA_CORRCT
    color = pow(color, vec3(1.0/GAMMA_VAL));
    #endif
    FragColor = vec4(color, 1.0);
} 
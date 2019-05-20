#version 460 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

#define LIGHT_MAX 10

struct LightDir {
	vec3 dir;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct LightPoint {
	vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct LightSpot {
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

struct LightUse {
    int dirs;
    int points;
    int spots;
};

in vec3 fg_pos;
in vec3 fg_color;
in vec2 fg_uv;
in vec3 fg_normal;

uniform vec3 camera_pos;
uniform Material material;
uniform LightDir dirs[LIGHT_MAX];
uniform LightPoint points[LIGHT_MAX];
uniform LightSpot spots[LIGHT_MAX];
uniform LightUse uses;

out vec4 FragColor;

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

vec3 calc_dir(LightDir light, vec3 camera_dir, vec3 normal, vec3 diffuse_color, vec3 specular_color) {
    vec3 dir = normalize(light.dir);

	vec3 ambient = phong_ambient(light.ambient, 1.0, diffuse_color);
    vec3 diffuse = phong_diffuse(light.diffuse, 1.0, dir, normal, diffuse_color);
    vec3 specular = phong_specular(light.specular, 1.0, dir, normal, camera_dir, specular_color);

    return ambient + diffuse + specular;
}

vec3 calc_point(LightPoint light, vec3 camera_dir, vec3 normal, vec3 diffuse_color, vec3 specular_color)
{
    vec3 dir = normalize(fg_pos - light.pos);
    float distance = length(light.pos - fg_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 ambient = phong_ambient(light.ambient, attenuation, diffuse_color);
    vec3 diffuse = phong_diffuse(light.diffuse, attenuation, dir, normal, diffuse_color);
    vec3 specular = phong_specular(light.specular, attenuation, dir, normal, camera_dir, specular_color);

    return ambient + diffuse + specular;
}

vec3 calc_spot(LightSpot light, vec3 camera_dir, vec3 normal, vec3 diffuse_color, vec3 specular_color)
{
    vec3 dir = normalize(fg_pos - light.pos);
    float theta = dot(dir, normalize(light.dir));
    if(theta > light.outter) {       
        float distance = length(light.pos - fg_pos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        float intensity = clamp((theta - light.outter) / (light.inner - light.outter), 0.0, 1.0);   

        vec3 ambient = phong_ambient(light.ambient, attenuation * intensity, diffuse_color);
        vec3 diffuse = phong_diffuse(light.diffuse, attenuation * intensity, dir, normal, diffuse_color);
        vec3 specular = phong_specular(light.specular, attenuation * intensity, dir, normal, camera_dir, specular_color);

        return ambient + diffuse + specular;
    }
    else {
       return vec3(0.0);
    }
}

void main()
{
    vec3 diffuse_color = texture(material.diffuse, fg_uv).rgb;
    vec3 specular_color = texture(material.specular, fg_uv).rgb;
    vec3 camera_dir = normalize(camera_pos - fg_pos);
    vec3 normal = normalize(fg_normal);

	vec3 color = vec3(0.0);
    for (int i = 0; i < uses.dirs; ++i) {
        color += calc_dir(dirs[i], camera_dir, normal, diffuse_color, specular_color);
    }
    for (int i = 0; i < uses.points; ++i) {
        color += calc_point(points[i], camera_dir, normal, diffuse_color, specular_color);
    }
    for (int i = 0; i < uses.spots; ++i) {
        color += calc_spot(spots[i], camera_dir, normal, diffuse_color, specular_color);
    }

    FragColor = vec4(color, 1.0);
} 
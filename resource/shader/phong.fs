#version 460 core
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
	vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 fg_pos;
in vec3 fg_color;
in vec2 fg_uv;
in vec3 fg_normal;

uniform sampler2D tex;
uniform vec3 camera_pos;
uniform Light light;
uniform Material material;

out vec4 FragColor;

void main()
{
	vec3 ambient = light.ambient * material.ambient;

    vec3 light_dir = normalize(light.pos - fg_pos);
    float diffuse_fac = max(dot(light_dir, normalize(fg_normal)), 0.0);
	vec3 diffuse = light.diffuse * material.diffuse * diffuse_fac;

    vec3 camera_dir = normalize(camera_pos - fg_pos);
    vec3 reflect_dir = reflect(-light_dir, normalize(fg_normal));
    float specular_fac = pow(max(dot(camera_dir, reflect_dir), 0.0), material.shininess);
	vec3 specular = light.specular * material.specular * specular_fac;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
} 
#version 460 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
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

in vec3 fg_pos;
in vec3 fg_color;
in vec2 fg_uv;
in vec3 fg_normal;

uniform vec3 camera_pos;
uniform Light light;
uniform Material material;

out vec4 FragColor;

void main()
{
    vec3 diffuse_color = texture(material.diffuse, fg_uv).rgb;
    vec3 specular_color = texture(material.specular, fg_uv).rgb;

    vec3 ambient = light.ambient * diffuse_color;

    vec3 light_dir = normalize(light.pos - fg_pos);
    float theta = dot(light_dir, normalize(-light.dir));

    if(theta > light.outter) {       
        float intensity = clamp((theta - light.outter) / (light.inner - light.outter), 0.0, 1.0);   

        float distance = length(light.pos - fg_pos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

        vec3 light_dir = normalize(light.pos - fg_pos);
        float diffuse_fac = max(dot(light_dir, normalize(fg_normal)), 0.0);
        vec3 diffuse = light.diffuse * diffuse_color * diffuse_fac * attenuation * intensity;

        vec3 camera_dir = normalize(camera_pos - fg_pos);
        vec3 reflect_dir = reflect(-light_dir, normalize(fg_normal));
        float specular_fac = pow(max(dot(camera_dir, reflect_dir), 0.0), material.shininess);
        vec3 specular = light.specular * specular_color * specular_fac * attenuation * intensity;

        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
    else {
        FragColor = vec4(ambient, 1.0);
    }
} 
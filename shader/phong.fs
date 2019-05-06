#version 460 core
in vec3 fg_pos;
in vec3 fg_color;
in vec2 fg_uv;
in vec3 fg_normal;

uniform sampler2D tex;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 camera_pos;
uniform vec3 ambient_color;

out vec4 FragColor;

void main()
{
    float ambient_fac = 0.1;
    float specular_fac = 0.5;
    int specular_pow = 32;

    vec3 light_dir = normalize(light_pos - fg_pos);
    float diffuse_fac = max(dot(light_dir, normalize(fg_normal)), 0.0);

    vec3 camera_dir = normalize(camera_pos - fg_pos);
    vec3 reflect_dir = reflect(-light_dir, normalize(fg_normal));
    specular_fac = specular_fac * pow(max(dot(camera_dir, reflect_dir), 0.0), specular_pow);

    vec3 light = ambient_color * ambient_fac + diffuse_fac * light_color + specular_fac * light_color;
    FragColor = vec4(light * fg_color, 1.0);
} 
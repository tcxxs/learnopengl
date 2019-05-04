#version 460 core
in vec3 fg_pos;
in vec3 fg_color;
in vec2 fg_uv;
in vec3 fg_normal;

uniform sampler2D tex;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 ambient;

out vec4 FragColor;

void main()
{
    vec3 light_dir = normalize(light_pos - fg_pos);
    float diffuse_fac = max(dot(light_dir, normalize(fg_normal)), 0.0);
    vec3 factor = ambient + diffuse_fac * light_color;
    FragColor = vec4(factor * fg_color, 1.0);
} 
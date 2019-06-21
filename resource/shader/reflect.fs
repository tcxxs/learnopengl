#version 460 core

in vec3 fg_pos;
in vec3 fg_normal;

uniform vec3 camera_pos;
uniform samplerCube uf_cube;

out vec4 FragColor;

void main()
{
    vec3 camera_dir = normalize(fg_pos - camera_pos);
    vec3 reflect_dir = reflect(camera_dir, normalize(fg_normal));

    FragColor = vec4(texture(uf_cube, reflect_dir).rgb, 1.0);
} 
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
    vec3 refract_dir = refract(camera_dir, normalize(fg_normal), 0.6);

    if (gl_FragCoord.x > 400)
        FragColor = vec4(texture(uf_cube, reflect_dir).rgb, 1.0);
    else
        FragColor = vec4(texture(uf_cube, refract_dir).rgb, 1.0);
} 
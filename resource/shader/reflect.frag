#version 460 core

in VertexAttrs {
    vec3 pos;
    vec3 normal;
}vertex;

uniform Scene {
    vec3 camera;
    int lights;
}scene;
uniform samplerCube uf_cube;

out vec4 FragColor;

void main()
{
    vec3 camera_dir = normalize(vertex.pos - scene.camera);
    vec3 reflect_dir = reflect(camera_dir, normalize(vertex.normal));
    vec3 refract_dir = refract(camera_dir, normalize(vertex.normal), 0.6);

    if (gl_FragCoord.x > 400)
        FragColor = vec4(texture(uf_cube, reflect_dir).rgb, 1.0);
    else
        FragColor = vec4(texture(uf_cube, refract_dir).rgb, 1.0);
} 
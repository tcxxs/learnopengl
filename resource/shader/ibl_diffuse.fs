#version 460 core

#define MATH_PI 3.14159265359

in VertexAttrs {
    vec3 pos;
}vertex;

uniform samplerCube cube;

out vec4 color_out;

void main() {
    vec3 normal = normalize(vertex.pos);
    vec3 irradiance = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, normal);
    up = cross(normal, right);

    float step = 0.025;
    int num = 0;
    for (float phi = 0.0; phi < 2.0 * MATH_PI; phi += step) {
        for (float theta = 0.0; theta < 0.5 * MATH_PI; theta += step) {
            num++;
            float sint = sin(theta);
            float cost = cos(theta);
            vec3 tangent = vec3(sint * cos(phi),  sint * sin(phi), cost);
            vec3 world = mat3(right, up, normal) * tangent;
            irradiance += texture(cube, world).rgb * cost * sint;
        }
    }
    irradiance = (MATH_PI / float(num)) * irradiance;

    color_out = vec4(irradiance, 1.0);
}
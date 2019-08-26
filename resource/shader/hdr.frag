#version 460 core

#define GAMMA_CORRCT 1
#define GAMMA_VAL 2.2

in vec2 fg_uv;
uniform sampler2D scene;
uniform sampler2D bloom;
uniform float exposure;

out vec4 color_out;

void main()
{
    vec3 color = texture(scene, fg_uv).rgb + texture(bloom, fg_uv).rgb;
    vec3 mapped = vec3(1.0) - exp(-color * exposure);

    #if GAMMA_CORRCT
    mapped = pow(mapped, vec3(1.0/GAMMA_VAL));
    #endif
    color_out = vec4(mapped, 1.0);
}
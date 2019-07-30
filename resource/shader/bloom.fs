#version 460 core

#define GAMMA_CORRCT 1
#define GAMMA_VAL 2.2

in vec2 fg_uv;
uniform sampler2D scene;
uniform sampler2D bloom;

out vec4 color_out;

void main()
{
    vec3 color = texture(scene, fg_uv).rgb;
    #if GAMMA_CORRCT
    color = pow(color, vec3(GAMMA_VAL));
    #endif

    #if GAMMA_CORRCT
    color = pow(color, vec3(1.0/GAMMA_VAL));
    #endif
    color_out = vec4(color, 1.0);
}
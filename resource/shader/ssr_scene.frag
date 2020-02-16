#version 460 core

#define SSR_METALLIC 0.3

in vec2 fg_uv;
uniform sampler2D ssr_uv;
uniform sampler2D ssr_reflect;
uniform sampler2D ssr_blur;
uniform sampler2D scene;

out vec4 color_out;

void main()
{
    vec4 color = texture(scene, fg_uv);
    vec4 data = texture(ssr_uv, fg_uv);
    vec2 uv = data.xy;
    float roughness = data.z;
    float metallic = data.w;
    if (metallic < SSR_METALLIC) {
        color_out = color;
        return;
    }

    vec4 ref = texture(ssr_reflect, fg_uv);
    vec4 blur = texture(ssr_blur, fg_uv);
    vec3 ssr = mix(ref.rgb, blur.rgb, 1 - roughness) * metallic;
    color_out.rgb = mix(color.rgb, ssr, ref.a);
    color_out.a = 1;
}
#version 460 core

#define VIEW_NEAR 0.1
#define VIEW_FAR 100.0

in vec2 fg_uv;

uniform sampler2D scene;
uniform sampler2D depth;
uniform float focus_dis;
uniform float focus_range;
uniform int coc_radius;

out vec4 color_out;

const int disc_num = 22;
const vec3 disc_samplers[22] = vec3[](						
    vec3(0, 0, 0),
    vec3(0.53333336, 0, 0.5333),
    vec3(0.3325279, 0.4169768, 0.5333),
    vec3(-0.11867785, 0.5199616, 0.5333),
    vec3(-0.48051673, 0.2314047, 0.5333),
    vec3(-0.48051673, -0.23140468, 0.5333),
    vec3(-0.11867763, -0.51996166, 0.5333),
    vec3(0.33252785, -0.4169769, 0.5333),
    vec3(1, 0, 1.0),
    vec3(0.90096885, 0.43388376, 1.0),
    vec3(0.6234898, 0.7818315, 1.0),
    vec3(0.22252098, 0.9749279, 1.0),
    vec3(-0.22252095, 0.9749279, 1.0),
    vec3(-0.62349, 0.7818314, 1.0),
    vec3(-0.90096885, 0.43388382, 1.0),
    vec3(-1, 0, 1.0),
    vec3(-0.90096885, -0.43388376, 1.0),
    vec3(-0.6234896, -0.7818316, 1.0),
    vec3(-0.22252055, -0.974928, 1.0),
    vec3(0.2225215, -0.9749278, 1.0),
    vec3(0.6234897, -0.7818316, 1.0),
    vec3(0.90096885, -0.43388376, 1.0)
);

float linear_depth(float z) {
    z = z * 2.0 - 1.0;
    z = (2.0 * VIEW_NEAR * VIEW_FAR) / (VIEW_FAR + VIEW_NEAR - z * (VIEW_FAR - VIEW_NEAR));
    return z;
}

float calc_coc(vec2 uv) {
    vec2 size = textureSize(depth, 0);
    size = 1.0 / size;

    float coc[4];
    vec4 offset = size.xyxy * vec2(-0.5, 0.5).xxyy;
    coc[0] = texture(depth, uv + offset.xy).x;
    coc[1] = texture(depth, uv + offset.zy).x;
    coc[2] = texture(depth, uv + offset.xw).x;
    coc[3] = texture(depth, uv + offset.zw).x;

    float cmin = 1;
    float cmax = -1;
    for (int i = 0; i < 4; ++i) {
        coc[i] = (linear_depth(coc[i]) - focus_dis) / focus_range;
        coc[i] = clamp(coc[i], -1, 1);
        cmin = min(cmin, coc[i]);
        cmax = max(cmax, coc[i]);
    }

    return cmax >= -cmin ? cmax : cmin;
}

float coc_weight(float coc, float length) {
    return clamp(coc * coc / length, 0, 1);
}

float color_weight(vec3 color) {
    return 1 / (1 + max(max(color.r, color.g), color.b));
}

vec3 calc_color(vec2 uv) {
    vec2 size = textureSize(scene, 0);
    size = 1.0 / size;

    vec3 color[4];
    vec4 offset = size.xyxy * vec2(-0.5, 0.5).xxyy;
    color[0] = texture(scene, uv + offset.xy).rgb;
    color[1] = texture(scene, uv + offset.zy).rgb;
    color[2] = texture(scene, uv + offset.xw).rgb;
    color[3] = texture(scene, uv + offset.zw).rgb;

    vec3 total = vec3(0);
    float weight = 0;
    for (int i = 0; i < 4; ++i) {
        float w = color_weight(color[i]);
        total += color[i] * w;
        weight += w;
    }
    if (weight > 0)
        return total / weight;
    else
        return vec3(0);
}

void main() {
    float coc = calc_coc(fg_uv);

    vec3 fg_color = vec3(0.0);
    float fg_weight = 0;
    vec3 bg_color = vec3(0.0);
    float bg_weight = 0;
    vec2 size = textureSize(scene, 0);
    size = 1.0 / size;
    for (int i = 0; i < disc_num; i++) {
        vec2 uv = fg_uv + disc_samplers[i].xy * size * coc_radius;
        float c = calc_coc(uv);
        if (coc > 0) {
            float w = coc_weight(max(0, c), disc_samplers[i].z);
            bg_color += calc_color(uv) * w;
            bg_weight += w;
        }
        else {
            float w = coc_weight(-min(0, c), disc_samplers[i].z);
            fg_color += calc_color(uv) * w;
            fg_weight += w;
        }
    }

    if (fg_weight > 0)
        fg_color /= fg_weight;
    if (bg_weight > 0)
        bg_color /= bg_weight;
    float gfac = min(1, fg_weight / disc_num);
    color_out.rgb = mix(bg_color, fg_color, gfac);

    float cfac = smoothstep(0.1, 1, abs(coc));
    color_out.a = cfac + gfac - cfac * gfac;
}
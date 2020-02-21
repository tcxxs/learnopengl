#version 460 core

#define MATH_EPS 0.00001
#define VIEW_NEAR 0.1
#define SPACE_VIEW 1

#define SSR_DISTANCE 15
#define SSR_RESOLVE 0.5
#define SSR_THICKNESS 0.001
#define SSR_FINDSTEPS 10

struct GBuffer {
    sampler2D position;
    sampler2D normal;
    sampler2D pbr;
};

in vec2 fg_uv;
uniform GBuffer gbuffer;
uniform sampler2D scene;
#if SPACE_VIEW
uniform MatrixVP {
    mat4 view;
    mat4 proj;
};
#endif

out vec4 color_uv;
out vec4 color_reflect;

void main()
{
    color_uv = vec4(0);
    color_reflect = vec4(0);

    vec2 tex_size  = textureSize(gbuffer.position, 0).xy;
    vec4 fg_pos = texture(gbuffer.position, fg_uv);
    if (length(fg_pos.xyz) < MATH_EPS && abs(fg_pos.w - 1.0) < MATH_EPS) {
        return;
    }
    vec3 ray_cast = normalize(fg_pos.xyz);
    vec3 normal = normalize(texture(gbuffer.normal, fg_uv).xyz);
    if (dot(normal, ray_cast) > 0) {
        return;
    }
    vec3 ray_reflect = normalize(reflect(ray_cast, normal));

    vec3 start_pos = fg_pos.xyz;
    vec2 start_scr = fg_uv * tex_size;
    vec3 end_pos = start_pos.xyz + ray_reflect * SSR_DISTANCE;
    // 在透视投影的时候，如果超出near面，会发生跳变
    if (end_pos.z > 0) {
        float scale = (-VIEW_NEAR - start_pos.z) / (end_pos.z - start_pos.z);
        end_pos = start_pos.xyz + ray_reflect * SSR_DISTANCE * scale;
    }
    vec4 end_proj = proj * vec4(end_pos, 1);
    end_proj.xyz /= end_proj.w;
    vec2 end_scr = (end_proj.xy * 0.5 + 0.5) * tex_size;
    // 如果路径超出了屏幕，缩小，提高精度
    if (end_scr.x < 0 || end_scr.x > tex_size.x || end_scr.y < 0 || end_scr.y > tex_size.y) {
        vec2 scale = vec2(1.0);
        if (end_scr.x < 0 || end_scr.x > tex_size.x) {
            scale.x = clamp(end_scr.x, 0, tex_size.x);
            scale.x = (scale.x - start_scr.x) / (end_scr.x - start_scr.x);
        }
        if (end_scr.y < 0 || end_scr.y > tex_size.y) {
            scale.y = clamp(end_scr.y, 0, tex_size.y);
            scale.y = (scale.y - start_scr.y) / (end_scr.y - start_scr.y);
        }
        // 获取屏幕插值
        float inter = min(scale.x, scale.y);
        end_scr = mix(start_scr, end_scr, inter);
        // 获取z插值，反推end_pos
        float z = (start_pos.z * end_pos.z) / min(mix(end_pos.z, start_pos.z, inter), -MATH_EPS);
        inter = clamp((z - start_pos.z) / (end_pos.z - start_pos.z), 0, 1);
        end_pos = mix(start_pos, end_pos, inter);
    }

    // 保证每次march至少跨越1个像素
    vec2 march_delta = vec2(end_scr.x - start_scr.x, end_scr.y - start_scr.y);
    bool march_x = abs(march_delta.x) > abs(march_delta.y) ? true : false;
    float march_iter = march_x ? abs(march_delta.x) : abs(march_delta.y);
    march_iter *= SSR_RESOLVE;
    if (march_iter < 1) {
        return;
    }
    vec2 march_inc = march_delta / march_iter;

    vec2 now_scr = start_scr;
    vec2 now_uv = vec2(0);
    vec3 now_pos = vec3(0);
    vec3 prev_pos = start_pos;
    float find_prev = 0;
    float find_cur = 0;
    float find_depth = 0;
    float find_diff = 0;
    bool find_hit = false;
    // 这里是比较难的，如果thickness范围大，resolve步进小，很有可能在marching的时候和起始点碰上了
    // 如果thickness小，resolve步进大，则在marching末端的时候，由于步进过大，容易穿过一些物体
    for (int i = 0; i < int(march_iter); ++i) {
        now_scr += march_inc;
        now_uv = now_scr / tex_size;
        now_pos = texture(gbuffer.position, now_uv).xyz;
        // 避免连续平面反射自己
        if (length(now_pos - prev_pos) > SSR_THICKNESS * 1.5) {
            prev_pos = now_pos;
        } else {
            continue;
        }

        vec2 inter = (now_scr - start_scr) / march_delta;
        find_cur = clamp(march_x ? inter.x : inter.y, 0, 1);
        find_depth = (start_pos.z * end_pos.z) / min(mix(end_pos.z, start_pos.z, find_cur), -MATH_EPS);
        find_diff = abs(find_depth - now_pos.z);
        if (find_diff > 0 && find_diff < SSR_THICKNESS) {
            find_hit = true;
            break;
        }
        else {
            find_prev = find_cur;
        }
    }
    if (!find_hit) {
        return;
    }

    find_cur = find_prev + ((find_cur - find_prev) / 2);
    for (int i = 0; i < SSR_FINDSTEPS; ++i) {
        now_scr = mix(start_scr, end_scr, find_cur);
        now_uv = now_scr / tex_size;
        now_pos = texture(gbuffer.position, now_uv).xyz;

        find_depth = (start_pos.z * end_pos.z) / min(mix(end_pos.z, start_pos.z, find_cur), -MATH_EPS);
        find_diff = abs(find_depth - now_pos.z);
        if (find_diff > 0 && find_diff < SSR_THICKNESS) {
            find_cur = find_prev + ((find_cur - find_prev) / 2);
        }
        else {
            float temp = find_cur;
            find_cur = find_cur + ((find_cur - find_prev) / 2);
            find_prev = temp;
        }
    }

    float vis = 1;
    vis *= 1 - max(dot(-ray_cast, ray_reflect), 0);
    vis *= 1 - clamp(abs(find_diff) / SSR_THICKNESS / 10, 0, 1);
    vis *= 1 - clamp(length(now_pos - fg_pos.xyz) / SSR_DISTANCE, 0, 1);
    vis = clamp(vis, 0, 1);

    color_uv = vec4(now_uv, texture(gbuffer.pbr, fg_uv).rg);
    color_reflect.rgb = texture(scene, now_uv).rgb;
    color_reflect.a = vis;
}
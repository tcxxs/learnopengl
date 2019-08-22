#version 460 core

#define MATH_PI 3.14159265359
#define MATH_EPS 0.0001

in vec2 fg_uv;

uniform int samples;

out vec4 color_out;

float radical_inverse(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    // 0x100000000
    return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley_sequence(uint i, uint n) {
    return vec2(float(i) / float(n), radical_inverse(i));
}

vec3 importance_ggx(vec2 rand, vec3 normal, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
	
    float phi = 2.0 * MATH_PI * rand.x;
    float cost = sqrt((1.0 - rand.y) / (1.0 + (a2 - 1.0) * rand.y));
    float sint = sqrt(1.0 - cost*cost);
    vec3 halfdir = vec3(cos(phi) * sint, sin(phi) * sint, cost);

    vec3 up = vec3(0.0, 0.0, 1.0);
    if (abs(normal.z - 1.0) < MATH_EPS)
        up = vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);
    halfdir = mat3(tangent, bitangent, normal) * halfdir;

    return normalize(halfdir);
}

float G_schlick_ggx(vec3 vector, vec3 normal, float roughness) {
    float a = roughness * roughness;
    float k = a / 2.0;
    float theta   = max(dot(normal, vector), 0.0);

    return theta / (theta * (1.0 - k) + k);
}

float G_smith(vec3 camdir, vec3 lightdir, vec3 normal, float roughness) {
    float ggx1  = G_schlick_ggx(lightdir, normal, roughness);
    float ggx2  = G_schlick_ggx(camdir, normal, roughness);

    return ggx1 * ggx2;
}

vec2 brdf_lut(float cost, float roughness) {
    vec3 normal = vec3(0.0, 0.0, 1.0);
    vec3 camdir = vec3(sqrt(1.0 - cost*cost), 0.0, cost);
    float scale = 0.0;
    float bias = 0.0;

    for(int i = 0; i < samples; ++i) {
        vec2 rand = hammersley_sequence(i, samples);
        vec3 halfdir  = importance_ggx(rand, normal, roughness);
        vec3 lightdir  = normalize(2.0 * dot(camdir, halfdir) * halfdir - camdir);

        float cosnl = lightdir.z;
        if(cosnl > 0.0) {
            float cosnh = max(halfdir.z, 0.0);
            float cosvh = max(dot(camdir, halfdir), 0.0);
            float geo = G_smith(camdir, lightdir, normal, roughness);
            float geofac = (geo * cosvh) / (cosnh * cost);
            float ffac = pow(1.0 - cosvh, 5.0);

            scale += (1.0 - ffac) * geofac;
            bias += ffac * geofac;
        }
    }

    scale /= float(samples);
    bias /= float(samples);
    return vec2(scale, bias);
}

void main() {
    vec2 lut = brdf_lut(fg_uv.x, fg_uv.y);
    color_out = vec4(lut, 0.0, 1.0);
}
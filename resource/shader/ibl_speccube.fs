#version 460 core

#define MATH_PI 3.14159265359
#define MATH_EPS 0.00001

in VertexAttrs {
    vec3 pos;
}vertex;

uniform int samples;
uniform samplerCube cube;
uniform float roughness;

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

vec3 importance_ggx(vec2 rand, vec3 normal) {
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

float D_ggx(float cost) {
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (cost * cost * (a2 - 1.0) + 1.0);

    return a2 / (MATH_PI * denom * denom);
}

float pdf_mipmap(vec3 normal, vec3 camdir, vec3 halfdir) {
    if (abs(roughness) < MATH_EPS)
        return 0.0;
    
    float cost = max(dot(normal, halfdir), 0.0);
    float denom = 4.0 * max(dot(halfdir, camdir), 0.0) + MATH_EPS;
    float pdf = (D_ggx(cost) * cost / denom) + MATH_EPS;

    float size = textureSize(cube, 0).x;
    // 每个像素表示多少单位球面积
    float texel  = 4.0 * MATH_PI / (6.0 * size * size);
    // 在roughness下，根据pdf，每个采样点的uv步长
    float delta = 1.0 / (float(samples) * pdf);
    // delta/texel就等于，每单位球面积表示多少像素
    // 相当于，步长越大，跨越像素多，就需要越大的mipmap
    return 0.5 * log2(delta / texel);
}

void main() {
    vec3 normal = normalize(vertex.pos);    
    vec3 camdir = normal;

    vec3 color = vec3(0.0);
    float weight = 0.0;   
    for(int i = 0; i < samples; ++i) {
        vec2 rand = hammersley_sequence(i, samples);
        vec3 halfdir  = importance_ggx(rand, normal);
        vec3 light  = normalize(2.0 * dot(camdir, halfdir) * halfdir - camdir);

        float cost = dot(normal, light);
        if(cost > 0.0) {
            float mip = pdf_mipmap(normal, camdir, halfdir);
            color += textureLod(cube, light, mip).rgb * cost;
            weight += cost;
        }
    }

    color_out = vec4(color / weight, 1.0);
}
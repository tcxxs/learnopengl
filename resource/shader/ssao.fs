#version 460 core

#define GAMMA_CORRCT 1
#define GAMMA_VAL 2.2

#define WINDOW_WIDTH 1600.0
#define WINDOW_HEIGHT 900.0

#define SAMPLE_SIZE 64
#define SAMPLE_RADIUS 0.2

struct GBuffer {
    sampler2D position;
    sampler2D normal;
};

in vec2 fg_uv;

uniform MatrixVP {
    mat4 view;
    mat4 proj;
};
uniform GBuffer gbuffer;
uniform sampler2D noise;
uniform vec3 samples[SAMPLE_SIZE];

out float color_out;

void main() {
    vec4 position = texture(gbuffer.position, fg_uv);
    vec3 pos = position.rgb;
    float depth = position.a;

    vec3 normal = texture(gbuffer.normal, fg_uv).rgb;

    vec2 noise_size = textureSize(noise, 0);
    vec2 noise_scale = vec2(WINDOW_WIDTH / noise_size.x, WINDOW_HEIGHT / noise_size.y);
    vec3 noise_vec = texture(noise, fg_uv * noise_scale).xyz;

    vec3 t = normalize(noise_vec - normal * dot(noise_vec, normal));
    vec3 b = cross(normal, t);
    mat3 tbn = mat3(t, b, normal);

    float occlusion = 0.0;
    for(int i = 0; i < SAMPLE_SIZE; ++i) {
        vec3 sp = tbn * samples[i];
        sp = pos + sp * SAMPLE_RADIUS; 

        vec4 offset = vec4(sp, 1.0);
        offset = proj * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sdep = -texture(gbuffer.position, offset.xy).a;
        float range = smoothstep(0.0, 1.0, SAMPLE_RADIUS / abs(pos.z - sdep));
        occlusion += (sdep >= sp.z ? 1.0 : 0.0) * range; 
    }

    occlusion = 1.0 - (occlusion / SAMPLE_SIZE);
    color_out = occlusion;
}

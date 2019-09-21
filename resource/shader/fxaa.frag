#version 460 core

// Trims the algorithm from processing darks.
//   0.0833 - upper limit (default, the start of visible unfiltered edges)
//   0.0625 - high quality (faster)
//   0.0312 - visible limit (slower)
// The minimum amount of local contrast required to apply algorithm.
//   0.333 - too little (faster)
//   0.250 - low quality
//   0.166 - default
//   0.125 - high quality 
//   0.063 - overkill (slower)
#define CONTRAST_THRESHOLD 0.0625
#define CONTRAST_RELATIVE 0.125

struct luminance {
    float m, n, e, s, w;
    float ne, nw, se, sw;
    float highest, lowest, contrast;
};

struct edge {
    bool horizontal;
    float pixelstep;
};

in vec2 fg_uv;
uniform sampler2D frame;

out vec4 color_out;

vec2 texel;
void init() {
    texel = 1.0 / textureSize(frame, 0);
}

float sample_luminance(vec2 uv, vec2 offset) {
    uv += texel * offset;
    vec4 color = texture(frame, uv);
    return color.g;
}

luminance sample_neighbor(vec2 uv) {
    luminance l;
    l.m = sample_luminance(uv, vec2(0, 0));
    l.n = sample_luminance(uv, vec2(0, 1));
    l.e = sample_luminance(uv, vec2(1, 0));
    l.s = sample_luminance(uv, vec2(0, -1));
    l.w = sample_luminance(uv, vec2(-1, 0));
    
    l.ne = sample_luminance(uv, vec2(1, 1));
    l.nw = sample_luminance(uv, vec2(-1, 1));
    l.se = sample_luminance(uv, vec2(1, -1));
    l.sw = sample_luminance(uv, vec2(-1, -1));
    
    l.highest = max(max(max(max(l.n, l.e), l.s), l.w), l.m);
    l.lowest = min(min(min(min(l.n, l.e), l.s), l.w), l.m);
    l.contrast = l.highest - l.lowest;
    return l;
}

bool skip_pixel(luminance l) {
    float threshold = max(CONTRAST_THRESHOLD, CONTRAST_RELATIVE * l.highest);
    return l.contrast < threshold;
}

float blend_factor(luminance l) {
    float avg = 2 * (l.n + l.e + l.s + l.w);
    avg += l.ne + l.nw + l.se + l.sw;
    avg *= 1.0 / 12;

    float fac = abs(avg - l.m);
    fac = clamp(fac / l.contrast, 0.0, 1.0);
    fac = smoothstep(0.0, 1.0, fac);
    return fac * fac;
}

edge determine_edge(luminance l) {
    edge e;
    float horizontal =
        abs(l.n + l.s - 2 * l.m) * 2 +
        abs(l.ne + l.se - 2 * l.e) +
        abs(l.nw + l.sw - 2 * l.w);
    float vertical =
        abs(l.e + l.w - 2 * l.m) * 2 +
        abs(l.ne + l.nw - 2 * l.n) +
        abs(l.se + l.sw - 2 * l.s);
    e.horizontal = horizontal >= vertical;
    e.pixelstep = e.horizontal ? texel.y : texel.x;

    float postive = e.horizontal ? l.n : l.e;
    float nagetive = e.horizontal ? l.s : l.w;
    postive = abs(postive - l.m);
    nagetive = abs(nagetive - l.m);
    if (postive < nagetive) {
        e.pixelstep = -e.pixelstep;
    }
    return e;
}

vec4 fxaa(vec2 uv) {
    luminance l = sample_neighbor(uv);
    if (skip_pixel(l)) {
        return texture(frame, uv);
    }

    float blend = blend_factor(l);
    edge e = determine_edge(l);
    if (e.horizontal) {
        uv.y += e.pixelstep * blend;
    }
    else {
        uv.x += e.pixelstep * blend;
    }

    return texture(frame, uv);
}

void main() {
    init();
    color_out = fxaa(fg_uv);
}
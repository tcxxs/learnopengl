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
// This can effect sharpness.
//   1.00 - upper limit (softer)
//   0.75 - default amount of filtering
//   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
//   0.25 - almost off
//   0.00 - completely off
#define CONTRAST_THRESHOLD 0.0625
#define CONTRAST_RELATIVE 0.125
#define SUBPIXEL_BLEND 1.0

struct luminance {
    float m, n, e, s, w;
    float ne, nw, se, sw;
    float highest, lowest, contrast;
};

struct edge {
    bool horizontal;
    float pixelstep;
    float opposite;
    float gradient;
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
    return dot(color.rgb, vec3(0.2126729,  0.7151522, 0.0721750));
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

float blend_region(luminance l) {
    float avg = 2 * (l.n + l.e + l.s + l.w);
    avg += l.ne + l.nw + l.se + l.sw;
    avg *= 1.0 / 12;

    float fac = abs(avg - l.m);
    fac = clamp(fac / l.contrast, 0.0, 1.0);
    fac = smoothstep(0.0, 1.0, fac);
    return fac * fac * SUBPIXEL_BLEND;
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

    float pluminance = e.horizontal ? l.n : l.e;
    float nluminance = e.horizontal ? l.s : l.w;
    float pgradient = abs(pluminance - l.m);
    float ngradient = abs(nluminance - l.m);
    if (pgradient < ngradient) {
        e.pixelstep = -e.pixelstep;
        e.opposite = nluminance;
        e.gradient = ngradient;
    }
    else {
        e.opposite = pluminance;
        e.gradient = pgradient;
    }
    return e;
}

float blend_edge(luminance l, edge e, vec2 uv) {
	vec2 edge_uv = uv;
    vec2 edge_step;
    if (e.horizontal) {
        edge_uv.y += e.pixelstep * 0.5;
        edge_step = vec2(texel.x, 0.0);
    }
    else {
        edge_uv.x += e.pixelstep * 0.5;
        edge_step = vec2(0.0, texel.y);
    }
    
    float edge_luminance = (l.m + e.opposite) * 0.5;
    float threshold = e.gradient * 0.25;
    vec2 pnext = edge_uv;
    float pdelta;
    bool pend = false;
    for (int i = 0; i < 9 && !pend; i++) {
        pnext += edge_step;
        pdelta = sample_luminance(pnext, vec2(0.0)) - edge_luminance;
        pend = abs(pdelta) >= threshold;
    }
    vec2 nnext = edge_uv;
    float ndelta;
    bool nend = false;
    for (int i = 0; i < 9 && !nend; i++) {
        nnext -= edge_step;
        ndelta = sample_luminance(nnext, vec2(0.0)) - edge_luminance;
        nend = abs(ndelta) >= threshold;
    }

    float pdis, ndis;
    if (e.horizontal) {
        pdis = pnext.x - uv.x;
        ndis = uv.x - nnext.x;
    }
    else {
        pdis = pnext.y - uv.y;
        ndis = uv.y - nnext.y;
    }
    float sdis;
    bool sdelta;
    if (pdis <= ndis) {
        sdis = pdis;
        sdelta = pdelta >= 0;
    }
    else {
        sdis = ndis;
        sdelta = ndelta >= 0;
    }

    if (sdelta == (l.m - edge_luminance >= 0)) {
        return 0.0;
    }
    else {
        return 0.5 - sdis / (pdis + ndis);
    }
}

vec4 fxaa(vec2 uv) {
    //return texture(frame, uv);
    luminance l = sample_neighbor(uv);
    if (skip_pixel(l)) {
        return texture(frame, uv);
    }

    float fregion = blend_region(l);
    edge e = determine_edge(l);
    float fedge = blend_edge(l, e, uv);
    float fac = max(fregion, fedge);
    if (e.horizontal) {
        uv.y += e.pixelstep * fac;
    }
    else {
        uv.x += e.pixelstep * fac;
    }

    return texture(frame, uv);
}

void main() {
    init();
    color_out = fxaa(fg_uv);
}
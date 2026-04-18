/* sdf.h — campi distantiae signatae: primitiva + operationes CSG. */
#ifndef PORTRAIT_SDF_H
#define PORTRAIT_SDF_H

#include "commonia.h"

/* ----- Operationes fundamentales ----- */
static inline float sdf_unio       (float a, float b) { return fminf(a, b); }
static inline float sdf_subtractio (float a, float b) { return fmaxf(a, -b); }
static inline float sdf_intersectio(float a, float b) { return fmaxf(a, b); }

/* Unio lenis — parametrum k ∈ (0, 1] moderatur mixtura */
static inline float sdf_unio_lenis(float a, float b, float k) {
    float h = fmaxf(k - fabsf(a - b), 0.0f) / (k + 1e-20f);
    return fminf(a, b) - h * h * k * 0.25f;
}

static inline float sdf_subtractio_lenis(float a, float b, float k) {
    float h = fmaxf(k - fabsf(a + b), 0.0f) / (k + 1e-20f);
    return fmaxf(a, -b) + h * h * k * 0.25f;
}

/* ----- Primitiva 2D ----- */

static inline float sdf_circulus(vec2 p, float r) {
    return v2_len(p) - r;
}

static inline float sdf_ellipsis(vec2 p, float rx, float ry) {
    /* Pseudo-SDF IQ: negativum intus, nil ad marginem, positivum extra.
     * Praesidio originis — ad p=0 formula est 0/0; reddimus profunditatem maximam. */
    if (rx <= 1e-10f || ry <= 1e-10f)
        return 1.0f;
    float kx = p.x / rx;
    float ky = p.y / ry;
    float k1 = sqrtf(kx*kx + ky*ky);
    float k2 = sqrtf(
        (p.x*p.x)/(rx*rx*rx*rx)
        + (p.y*p.y)/(ry*ry*ry*ry)
    );
    if (k2 < 1e-10f)
        return -fminf(rx, ry);  /* interior profundissimum */
    return k1 * (k1 - 1.0f) / k2;
}

static inline float sdf_capsa(vec2 p, vec2 b) {
    vec2 d = v2_sub(v2_abs(p), b);
    vec2 m = v2(fmaxf(d.x, 0.0f), fmaxf(d.y, 0.0f));
    return v2_len(m) + fminf(fmaxf(d.x, d.y), 0.0f);
}

static inline float sdf_capsa_rotunda(vec2 p, vec2 b, float r) {
    return sdf_capsa(p, b) - r;
}

static inline float sdf_capsula(vec2 p, vec2 a, vec2 b, float r) {
    vec2 pa = v2_sub(p, a), ba = v2_sub(b, a);
    float h = clampf(v2_dot(pa, ba) / (v2_dot(ba, ba) + 1e-20f), 0.0f, 1.0f);
    return v2_len(v2_sub(pa, v2_mul(ba, h))) - r;
}

/* "Lens" (forma amygdalae) — oculi.  l = longitudo axis, a = acies */
static inline float sdf_lens(vec2 p, float l, float a) {
    if (a <= 0.0001f)
        a = 0.0001f;
    vec2 q = v2(fabsf(p.x), fabsf(p.y));
    float b = (a*a - l*l) / (2.0f * a);
    float h = sqrtf(fmaxf(l*l - b*b, 0.0f));
    /* Distantia ad duos arcus circularum superpositorum */
    if (h * q.x < b * q.y) {
        return v2_len(v2(q.x, q.y - h));
    } else {
        return v2_len(v2(q.x + b, q.y)) - (b + a);
    }
}

/* Arcus: arx super semicirculum; r = radius, t = crassitudo semi, angulus = apertura */
static inline float sdf_arcus(vec2 p, float apertura, float r, float t) {
    vec2 sc = v2(sinf(apertura), cosf(apertura));
    p.x = fabsf(p.x);
    if (sc.y * p.x > sc.x * p.y) {
        return v2_len(v2_sub(p, v2(sc.x * r, sc.y * r))) - t;
    } else {
        return fabsf(v2_len(p) - r) - t;
    }
}

/* Forma cordis — pro ore amantis vel aliis ornamentis */
static inline float sdf_cor(vec2 p, float s) {
    p.x = fabsf(p.x);
    if (p.y + p.x > s) {
        return sqrtf(
            (p.x - 0.25f * s) * (p.x - 0.25f * s)
            + (p.y - 0.75f * s) * (p.y - 0.75f * s)
        ) - 0.3535f * s;
    } else {
        float d1 = v2_len(v2_sub(p, v2(0.0f, s)));
        float d2 = v2_len(v2_sub(p, v2_mul(v2(1.0f, 1.0f), 0.5f * fmaxf(p.x + p.y, 0.0f))));
        return sqrtf(fminf(d1*d1, d2*d2)) * signf(p.x - p.y);
    }
}

/* Triangulum aequilaterum */
static inline float sdf_triangulum_aeq(vec2 p, float r) {
    const float k = 1.7320508f; /* sqrt 3 */
    p.x = fabsf(p.x) - r;
    p.y = p.y + r / k;
    if (p.x + k * p.y > 0.0f) {
        float nx = (p.x - k * p.y) * 0.5f;
        float ny = (-k * p.x - p.y) * 0.5f;
        p.x = nx;
        p.y = ny;
    }
    p.x -= clampf(p.x, -2.0f * r, 0.0f);
    return -v2_len(p) * signf(p.y);
}

/* Ellipsis inclinata: p in spatio mundi, centrum offset, anguli */
static inline float sdf_ellipsis_rotata(vec2 p, vec2 centrum, float rx, float ry, float angulus) {
    vec2 q = v2_rotatus(v2_sub(p, centrum), -angulus);
    return sdf_ellipsis(q, rx, ry);
}

/* Distortio SDF per scalam non-uniformem — utile ad formas ovalium */
static inline float sdf_ellipsis_distorta(
    vec2 p, float rx, float ry,
    float warp_x, float warp_y
) {
    /* warp_x, warp_y ∈ [-1, 1] — pinch */
    float sx = 1.0f + warp_x * (p.y / (ry + 1e-20f));
    float sy = 1.0f + warp_y * (p.x / (rx + 1e-20f));
    p.x /= sx;
    p.y /= sy;
    return sdf_ellipsis(p, rx, ry);
}

/* Bezier quadraticus — approximatio distantiae minimalis per N iter */
float sdf_bezier_quad(vec2 p, vec2 a, vec2 b, vec2 c);

/* Cobertura ex SDF + scala pixelum */
static inline float sdf_cobertura(float d) {
    return smoothstepf(0.6f, -0.6f, d);
}

#endif

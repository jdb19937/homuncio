#include "sdf.h"

/* Bezier quadraticus — approximatio distantiae per iterationem Newton.
 * Haec implementatio est ex analyse Inigo Quilez, simplificata. */
float sdf_bezier_quad(vec2 p, vec2 A, vec2 B, vec2 C) {
    vec2 a = v2_sub(B, A);
    vec2 b = v2_add(v2_sub(A, v2_mul(B, 2.0f)), C);
    vec2 c = v2_mul(a, 2.0f);
    vec2 d = v2_sub(A, p);
    float kk = 1.0f / (v2_dot(b, b) + 1e-20f);
    float kx = kk * v2_dot(a, b);
    float ky = kk * (2.0f * v2_dot(a, a) + v2_dot(d, b)) / 3.0f;
    float kz = kk * v2_dot(d, a);
    float res = 0.0f;
    float pp = ky - kx * kx;
    float q  = kx * (2.0f * kx * kx - 3.0f * ky) + kz;
    float h  = q * q + 4.0f * pp * pp * pp;
    if (h >= 0.0f) {
        h = sqrtf(h);
        float v1 = (-q + h) * 0.5f;
        float v2 = (-q - h) * 0.5f;
        float x_ = (v1 < 0 ? -powf(-v1, 1.0f/3.0f) : powf(v1, 1.0f/3.0f))
            + (v2 < 0 ? -powf(-v2, 1.0f/3.0f) : powf(v2, 1.0f/3.0f));
        float t = clampf(x_ - kx, 0.0f, 1.0f);
        vec2  qq = v2_add(d, v2_mul(v2_add(c, v2_mul(b, t)), t));
        res = v2_len(qq);
    } else {
        float z = sqrtf(-pp);
        float v = acosf(q / (pp * z * 2.0f)) / 3.0f;
        float m = cosf(v);
        float n = sinf(v) * 1.7320508f;
        float t1 = clampf((m + m) * z - kx, 0.0f, 1.0f);
        float t2 = clampf((-n - m) * z - kx, 0.0f, 1.0f);
        vec2 qa = v2_add(d, v2_mul(v2_add(c, v2_mul(b, t1)), t1));
        vec2 qb = v2_add(d, v2_mul(v2_add(c, v2_mul(b, t2)), t2));
        float ra = v2_dot(qa, qa);
        float rb = v2_dot(qb, qb);
        res = sqrtf(fminf(ra, rb));
    }
    return res;
}

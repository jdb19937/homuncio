/* commonia.h — mathematica fundamentalis, typi, macros, omnibus communia. */
#ifndef PORTRAIT_COMMONIA_H
#define PORTRAIT_COMMONIA_H

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#ifndef PORTRAIT_PI
#define PORTRAIT_PI 3.14159265358979323846f
#endif

#define PORTRAIT_TAU (2.0f * PORTRAIT_PI)

/* --- Vector struct 2D --- */
typedef struct {
    float x, y;
}vec2;

static inline vec2 v2(float x, float y) {
    vec2 v;
    v.x = x;
    v.y = y;
    return v;
}
static inline vec2 v2_add(vec2 a, vec2 b) { return v2(a.x + b.x, a.y + b.y); }
static inline vec2 v2_sub(vec2 a, vec2 b) { return v2(a.x - b.x, a.y - b.y); }
static inline vec2 v2_mul(vec2 a, float s) { return v2(a.x * s, a.y * s); }
static inline float v2_dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }
static inline float v2_len(vec2 a) { return sqrtf(a.x * a.x + a.y * a.y); }
static inline float v2_len2(vec2 a) { return a.x * a.x + a.y * a.y; }
static inline vec2 v2_abs(vec2 a) { return v2(fabsf(a.x), fabsf(a.y)); }
static inline vec2 v2_rotatus(vec2 a, float ang) {
    float c = cosf(ang), s = sinf(ang);
    return v2(a.x * c - a.y * s, a.x * s + a.y * c);
}

/* --- Scalar helpers --- */
static inline float clampf(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}
static inline float saturatef(float x) { return clampf(x, 0.0f, 1.0f); }
static inline int   clampi(int x, int lo, int hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}
static inline float mixf(float a, float b, float t) { return a + (b - a) * t; }
static inline float smoothstepf(float e0, float e1, float x) {
    float t = clampf((x - e0) / (e1 - e0 + 1e-20f), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
static inline float smootherstepf(float e0, float e1, float x) {
    float t = clampf((x - e0) / (e1 - e0 + 1e-20f), 0.0f, 1.0f);
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}
static inline float signf(float x) { return x < 0.0f ? -1.0f : (x > 0.0f ? 1.0f : 0.0f); }

/* --- 2D distance helpers --- */
static inline vec2  v2_max(vec2 a, vec2 b) { return v2(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y); }
static inline vec2  v2_min(vec2 a, vec2 b) { return v2(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y); }

#endif /* PORTRAIT_COMMONIA_H */

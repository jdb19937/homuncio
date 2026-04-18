#include "tabula.h"
#include <stdlib.h>
#include <string.h>

Tabula* tabula_nova(int w, int h) {
    Tabula* t = (Tabula*) calloc(1, sizeof(Tabula));
    if (!t)
        return NULL;
    t->w = w;
    t->h = h;
    t->pixels = (float*) calloc((size_t)w * (size_t)h * 4u, sizeof(float));
    if (!t->pixels) {
        free(t);
        return NULL;
    }
    return t;
}

void tabula_dele(Tabula* t) {
    if (!t)
        return;
    free(t->pixels);
    free(t);
}

void tabula_imple(Tabula* t, Color c) {
    if (!t)
        return;
    int n = t->w * t->h;
    for (int i = 0; i < n; i++) {
        t->pixels[i*4+0] = c.r;
        t->pixels[i*4+1] = c.g;
        t->pixels[i*4+2] = c.b;
        t->pixels[i*4+3] = c.a;
    }
}

static inline int in_bounds(const Tabula* t, int x, int y) {
    return x >= 0 && y >= 0 && x < t->w && y < t->h;
}

void tabula_pone(Tabula* t, int x, int y, Color c) {
    if (!in_bounds(t, x, y))
        return;
    int i = (y * t->w + x) * 4;
    t->pixels[i+0] = c.r;
    t->pixels[i+1] = c.g;
    t->pixels[i+2] = c.b;
    t->pixels[i+3] = c.a;
}

void tabula_misce(Tabula* t, int x, int y, Color c) {
    if (!in_bounds(t, x, y))
        return;
    int i = (y * t->w + x) * 4;
    float a = saturatef(c.a);
    float ia = 1.0f - a;
    t->pixels[i+0] = c.r * a + t->pixels[i+0] * ia;
    t->pixels[i+1] = c.g * a + t->pixels[i+1] * ia;
    t->pixels[i+2] = c.b * a + t->pixels[i+2] * ia;
    t->pixels[i+3] = a + t->pixels[i+3] * ia;
}

void tabula_misce_pondus(Tabula* t, int x, int y, Color c, float pondus) {
    c.a *= saturatef(pondus);
    tabula_misce(t, x, y, c);
}

Color tabula_lege(const Tabula* t, int x, int y) {
    if (!in_bounds(t, x, y))
        return color4(0, 0, 0, 0);
    int i = (y * t->w + x) * 4;
    return color4(t->pixels[i+0], t->pixels[i+1], t->pixels[i+2], t->pixels[i+3]);
}

void tabula_pinge_sdf(
    Tabula* t, vec2 centrum, float raggio, SdfProba f,
    void* ctx, Color c
) {
    if (!t || !f)
        return;
    int x0 = clampi((int) floorf(centrum.x - raggio) - 1, 0, t->w - 1);
    int y0 = clampi((int) floorf(centrum.y - raggio) - 1, 0, t->h - 1);
    int x1 = clampi((int) ceilf (centrum.x + raggio) + 1, 0, t->w - 1);
    int y1 = clampi((int) ceilf (centrum.y + raggio) + 1, 0, t->h - 1);
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            vec2 p = v2((float)x + 0.5f, (float)y + 0.5f);
            float d = f(p, ctx);
            /* cobertura ex SDF: intra 0.7 pixel transitio */
            float cov = smoothstepf(0.6f, -0.6f, d);
            if (cov > 0.0f) {
                Color cc = c;
                cc.a *= cov;
                tabula_misce(t, x, y, cc);
            }
        }
    }
}

/* SDF capsulae inter puncta a et b cum radio r */
typedef struct {
    vec2 a, b;
    float r;
}LineaCtx;
static float sdf_linea(vec2 p, void* ctx_) {
    LineaCtx* ctx = (LineaCtx*) ctx_;
    vec2 ab = v2_sub(ctx->b, ctx->a);
    vec2 ap = v2_sub(p,      ctx->a);
    float t = v2_dot(ap, ab) / (v2_dot(ab, ab) + 1e-10f);
    t = clampf(t, 0.0f, 1.0f);
    vec2 c = v2_add(ctx->a, v2_mul(ab, t));
    return v2_len(v2_sub(p, c)) - ctx->r;
}

void tabula_pinge_lineam(Tabula* t, vec2 a, vec2 b, float crassitudo, Color c) {
    LineaCtx ctx = { a, b, crassitudo * 0.5f };
    vec2 centrum = v2((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f);
    float raggio = v2_len(v2_sub(b, a)) * 0.5f + ctx.r + 2.0f;
    tabula_pinge_sdf(t, centrum, raggio, sdf_linea, &ctx, c);
}

typedef struct {
    vec2 c;
    float r;
}DiscusCtx;
static float sdf_discus(vec2 p, void* ctx_) {
    DiscusCtx* ctx = (DiscusCtx*) ctx_;
    return v2_len(v2_sub(p, ctx->c)) - ctx->r;
}

void tabula_pinge_discum(Tabula* t, vec2 centrum, float r, Color c) {
    DiscusCtx ctx = { centrum, r };
    tabula_pinge_sdf(t, centrum, r + 2.0f, sdf_discus, &ctx, c);
}

static inline uint8_t tof8(float v) {
    if (v <= 0.0f)
        return 0;
    if (v >= 1.0f)
        return 255;
    /* linear → sRGB approximato per gamma 1/2.2 */
    float s = powf(v, 1.0f / 2.2f);
    int i = (int)(s * 255.0f + 0.5f);
    return (uint8_t) clampi(i, 0, 255);
}

void tabula_ad_rgba8(const Tabula* t, uint8_t* out) {
    if (!t || !out)
        return;
    int n = t->w * t->h;
    for (int i = 0; i < n; i++) {
        float r = t->pixels[i*4+0];
        float g = t->pixels[i*4+1];
        float b = t->pixels[i*4+2];
        float a = t->pixels[i*4+3];
        out[i*4+0] = tof8(r);
        out[i*4+1] = tof8(g);
        out[i*4+2] = tof8(b);
        out[i*4+3] = (uint8_t) clampi((int)(a * 255.0f + 0.5f), 0, 255);
    }
}

Tabula* tabula_clona(const Tabula* t) {
    if (!t)
        return NULL;
    Tabula* n = tabula_nova(t->w, t->h);
    if (!n)
        return NULL;
    memcpy(n->pixels, t->pixels, (size_t)t->w * t->h * 4u * sizeof(float));
    return n;
}

void tabula_copia(Tabula* dst, const Tabula* src) {
    if (!dst || !src)
        return;
    if (dst->w != src->w || dst->h != src->h)
        return;
    memcpy(dst->pixels, src->pixels, (size_t)dst->w * dst->h * 4u * sizeof(float));
}

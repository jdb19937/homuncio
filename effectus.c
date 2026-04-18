#include "effectus.h"
#include "color.h"
#include "tumultus.h"
#include <stdlib.h>
#include <string.h>

static void fx_vignetta(Tabula* t, float vis) {
    int w = t->w, h = t->h;
    float cx = w * 0.5f, cy = h * 0.5f;
    float maxd = sqrtf(cx*cx + cy*cy);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float dx = (float)x - cx;
            float dy = (float)y - cy;
            float d = sqrtf(dx*dx + dy*dy) / maxd;
            float fac = 1.0f - vis * smoothstepf(0.55f, 1.05f, d);
            int i = (y * w + x) * 4;
            t->pixels[i+0] *= fac;
            t->pixels[i+1] *= fac;
            t->pixels[i+2] *= fac;
        }
    }
}

static void fx_granum(Tabula* t, float vis, float tempus) {
    int w = t->w, h = t->h;
    uint32_t semen = (uint32_t)(tempus * 1000.0f) ^ 0xFACADEu;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int i = y * w + x;
            float n = tumultus2_s((float)x * 1.3f, (float)y * 1.7f, semen);
            t->pixels[i*4+0] = clampf(t->pixels[i*4+0] + n * vis * 0.15f, 0, 1);
            t->pixels[i*4+1] = clampf(t->pixels[i*4+1] + n * vis * 0.15f, 0, 1);
            t->pixels[i*4+2] = clampf(t->pixels[i*4+2] + n * vis * 0.15f, 0, 1);
        }
    }
}

static void fx_scanlineae_crt(Tabula* t, float vis) {
    int w = t->w, h = t->h;
    for (int y = 0; y < h; y++) {
        float fac = (y & 1) ? (1.0f - 0.3f * vis) : 1.0f;
        for (int x = 0; x < w; x++) {
            int i = (y * w + x) * 4;
            t->pixels[i+0] *= fac;
            t->pixels[i+1] *= fac;
            t->pixels[i+2] *= fac;
        }
    }
}

static void fx_aber_chromatis(Tabula* t, float vis) {
    int w = t->w, h = t->h;
    Tabula* c = tabula_clona(t);
    if (!c)
        return;
    int shift = 1 + (int)(vis * 2.0f);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int xr = clampi(x - shift, 0, w - 1);
            int xb = clampi(x + shift, 0, w - 1);
            int i = (y * w + x) * 4;
            t->pixels[i+0] = c->pixels[(y*w+xr)*4+0];
            t->pixels[i+2] = c->pixels[(y*w+xb)*4+2];
        }
    }
    tabula_dele(c);
}

static const int bayer8[64] = {
    0, 32, 8, 40, 2, 34, 10, 42,
    48, 16, 56, 24, 50, 18, 58, 26,
    12, 44, 4, 36, 14, 46, 6, 38,
    60, 28, 52, 20, 62, 30, 54, 22,
    3, 35, 11, 43, 1, 33, 9, 41,
    51, 19, 59, 27, 49, 17, 57, 25,
    15, 47, 7, 39, 13, 45, 5, 37,
    63, 31, 55, 23, 61, 29, 53, 21,
};

static void fx_dithering_bayer(Tabula* t, float vis) {
    int w = t->w, h = t->h;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float bay = (float) bayer8[(y%8)*8 + (x%8)] / 64.0f - 0.5f;
            int i = (y * w + x) * 4;
            for (int k = 0; k < 3; k++) {
                float v = t->pixels[i+k] + bay * 0.1f * vis;
                t->pixels[i+k] = floorf(clampf(v, 0, 1) * 4.0f + 0.5f) / 4.0f;
            }
        }
    }
}

static void fx_dithering_fs(Tabula* t, float vis) {
    int w = t->w, h = t->h;
    (void)vis;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int i = (y * w + x) * 4;
            for (int k = 0; k < 3; k++) {
                float old = t->pixels[i+k];
                float nw  = (float)(old > 0.5f);
                t->pixels[i+k] = nw;
                float err = old - nw;
                if (x + 1 < w)
                    t->pixels[(y*w + x+1)*4 + k] += err * (7.0f/16.0f);
                if (y + 1 < h) {
                    if (x > 0)
                        t->pixels[((y+1)*w + x-1)*4 + k] += err * (3.0f/16.0f);
                    t->pixels[((y+1)*w + x  )*4 + k] += err * (5.0f/16.0f);
                    if (x + 1 < w)
                        t->pixels[((y+1)*w + x+1)*4 + k] += err * (1.0f/16.0f);
                }
            }
        }
    }
}

static void fx_halftone(Tabula* t, float vis) {
    int w = t->w, h = t->h;
    (void)vis;
    Tabula* c = tabula_clona(t);
    if (!c)
        return;
    int cell = 4;
    for (int y = 0; y < h; y += cell) {
        for (int x = 0; x < w; x += cell) {
            /* luminantia media cellae */
            float sum = 0.0f;
            int cnt = 0;
            for (int dy = 0; dy < cell && y+dy < h; dy++) {
                for (int dx = 0; dx < cell && x+dx < w; dx++) {
                    int i = ((y+dy) * w + x+dx) * 4;
                    sum += color_luminantia(color4(c->pixels[i], c->pixels[i+1], c->pixels[i+2], 1));
                    cnt++;
                }
            }
            float lum = sum / (float)cnt;
            /* Paint black circle with radius ∝ (1-lum) */
            float r_dot = (float)cell * 0.55f * (1.0f - lum);
            float ccx = (float)x + cell * 0.5f;
            float ccy = (float)y + cell * 0.5f;
            for (int dy = 0; dy < cell && y+dy < h; dy++) {
                for (int dx = 0; dx < cell && x+dx < w; dx++) {
                    float px = (float)(x + dx) + 0.5f;
                    float py = (float)(y + dy) + 0.5f;
                    float d = sqrtf((px-ccx)*(px-ccx) + (py-ccy)*(py-ccy));
                    float v = d < r_dot ? 0.0f : 1.0f;
                    int i = ((y+dy)*w + x+dx) * 4;
                    t->pixels[i+0] = v;
                    t->pixels[i+1] = v;
                    t->pixels[i+2] = v;
                }
            }
        }
    }
    tabula_dele(c);
}

static void fx_posterizatio(Tabula* t, float vis) {
    int gradus = (int)(8.0f - 6.0f * vis);
    if (gradus < 2)
        gradus = 2;
    int n = t->w * t->h;
    for (int i = 0; i < n; i++) {
        Color c = color4(t->pixels[i*4], t->pixels[i*4+1], t->pixels[i*4+2], 1);
        Color p = color_posterizata(c, gradus);
        t->pixels[i*4+0] = p.r;
        t->pixels[i*4+1] = p.g;
        t->pixels[i*4+2] = p.b;
    }
}

static void fx_lineae_prominentes(Tabula* t, float vis) {
    int w = t->w, h = t->h;
    float* gray = (float*) malloc((size_t)w * h * sizeof(float));
    for (int i = 0; i < w * h; i++)
        gray[i] = color_luminantia(color4(t->pixels[i*4], t->pixels[i*4+1], t->pixels[i*4+2], 1));
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            float gx = gray[(y-1)*w + x + 1] - gray[(y-1)*w + x - 1]
                + 2.0f * (gray[y*w + x + 1] - gray[y*w + x - 1])
                + gray[(y+1)*w + x + 1] - gray[(y+1)*w + x - 1];
            float gy = gray[(y+1)*w + x - 1] - gray[(y-1)*w + x - 1]
                + 2.0f * (gray[(y+1)*w + x] - gray[(y-1)*w + x])
                + gray[(y+1)*w + x + 1] - gray[(y-1)*w + x + 1];
            float mag = sqrtf(gx*gx + gy*gy);
            if (mag > 0.2f) {
                int i = (y*w + x) * 4;
                float a = vis * saturatef((mag - 0.2f) * 2.0f);
                t->pixels[i+0] *= (1.0f - a);
                t->pixels[i+1] *= (1.0f - a);
                t->pixels[i+2] *= (1.0f - a);
            }
        }
    }
    free(gray);
}

static void fx_nitor(Tabula* t, float vis, float tempus) {
    int w = t->w, h = t->h;
    float pulse = 0.85f + 0.25f * sinf(PORTRAIT_TAU * tempus);
    /* Box blur: pass 1 */
    Tabula* c = tabula_clona(t);
    if (!c)
        return;
    int r = 2;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float bright[3] = {0, 0, 0};
            int cnt = 0;
            for (int dy = -r; dy <= r; dy++) {
                for (int dx = -r; dx <= r; dx++) {
                    int xx = clampi(x + dx, 0, w - 1);
                    int yy = clampi(y + dy, 0, h - 1);
                    int i = (yy * w + xx) * 4;
                    float l = color_luminantia(color4(c->pixels[i], c->pixels[i+1], c->pixels[i+2], 1));
                    if (l > 0.7f) {
                        bright[0] += c->pixels[i];
                        bright[1] += c->pixels[i+1];
                        bright[2] += c->pixels[i+2];
                    }
                    cnt++;
                }
            }
            float inv = 1.0f / (float)cnt;
            int i = (y * w + x) * 4;
            /* Screen blend */
            float a = vis * pulse;
            t->pixels[i+0] = 1.0f - (1.0f - t->pixels[i+0]) * (1.0f - bright[0] * inv * a);
            t->pixels[i+1] = 1.0f - (1.0f - t->pixels[i+1]) * (1.0f - bright[1] * inv * a);
            t->pixels[i+2] = 1.0f - (1.0f - t->pixels[i+2]) * (1.0f - bright[2] * inv * a);
        }
    }
    tabula_dele(c);
}

void effectus_applica(Tabula* t, PostEffectus fx, float vis, float tempus) {
    vis = clampf(vis, 0.0f, 1.0f);
    switch (fx) {
    case FX_NULLUS: break;
    case FX_VIGNETTA:         fx_vignetta(t, vis); break;
    case FX_GRANUM:           fx_granum(t, vis, tempus); break;
    case FX_SCANLINEAE_CRT:   fx_scanlineae_crt(t, vis); break;
    case FX_ABER_CHROMATIS:   fx_aber_chromatis(t, vis); break;
    case FX_DITHERING_BAYER:  fx_dithering_bayer(t, vis); break;
    case FX_DITHERING_FS:     fx_dithering_fs(t, vis); break;
    case FX_HALFTONE:         fx_halftone(t, vis); break;
    case FX_POSTERIZATIO:     fx_posterizatio(t, vis); break;
    case FX_LINEAE_PROMINENTES: fx_lineae_prominentes(t, vis); break;
    case FX_NITOR:            fx_nitor(t, vis, tempus); break;
    default: break;
    }
}

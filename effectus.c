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
    Tabula* c = tabula_clona(t);
    if (!c)
        return;
    int r = 2;
    /* Ponderatio quadratica: solum pixel vere clari (l > 0.78) contribuunt,
     * ne bloom mollia omnem faciem muddat. */
    const float thr = 0.78f;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float bright[3] = {0, 0, 0};
            float wsum = 0.0f;
            for (int dy = -r; dy <= r; dy++) {
                for (int dx = -r; dx <= r; dx++) {
                    int xx = clampi(x + dx, 0, w - 1);
                    int yy = clampi(y + dy, 0, h - 1);
                    int i = (yy * w + xx) * 4;
                    float l = color_luminantia(color4(c->pixels[i], c->pixels[i+1], c->pixels[i+2], 1));
                    float excess = l - thr;
                    if (excess > 0.0f) {
                        float wp = excess * excess * 16.0f;
                        bright[0] += c->pixels[i]   * wp;
                        bright[1] += c->pixels[i+1] * wp;
                        bright[2] += c->pixels[i+2] * wp;
                        wsum += wp;
                    }
                }
            }
            if (wsum < 1e-4f)
                continue;
            int i = (y * w + x) * 4;
            float inv = 1.0f / wsum;
            /* Screen blend, dimidiatum ne hazy fiat */
            float a = vis * pulse * 0.5f;
            t->pixels[i+0] = 1.0f - (1.0f - t->pixels[i+0]) * (1.0f - bright[0] * inv * a);
            t->pixels[i+1] = 1.0f - (1.0f - t->pixels[i+1]) * (1.0f - bright[1] * inv * a);
            t->pixels[i+2] = 1.0f - (1.0f - t->pixels[i+2]) * (1.0f - bright[2] * inv * a);
        }
    }
    tabula_dele(c);
}

/* FX_PATINA: oxidatio aerea — tonus viridis-caeruleus in umbris, umbilicus aureus in lucibus */
static void fx_patina(Tabula* t, float vis) {
    int n = t->w * t->h;
    for (int i = 0; i < n; i++) {
        float r = t->pixels[i*4+0];
        float g = t->pixels[i*4+1];
        float b = t->pixels[i*4+2];
        float l = 0.299f * r + 0.587f * g + 0.114f * b;
        /* umbra: verdigris (0.28, 0.55, 0.42); lux: metallum obscurum (0.45, 0.40, 0.22) */
        float ru = 0.25f, gu = 0.55f, bu = 0.45f;
        float rl = 0.50f, gl = 0.42f, bl = 0.20f;
        /* Mix factor per lum: low lum → umbra verdigris, high lum → metallum */
        float pr = (1.0f - l) * ru + l * rl;
        float pg = (1.0f - l) * gu + l * gl;
        float pb = (1.0f - l) * bu + l * bl;
        /* Mix cum origine per vis */
        t->pixels[i*4+0] = mixf(r, pr, vis * 0.75f);
        t->pixels[i*4+1] = mixf(g, pg, vis * 0.75f);
        t->pixels[i*4+2] = mixf(b, pb, vis * 0.75f);
    }
}

/* FX_FRESCO: stylus Pompeianus — tonus terreus, mollitia, hint rimarum */
static void fx_fresco(Tabula* t, float vis) {
    int w = t->w, h = t->h;
    Tabula* c = tabula_clona(t);
    if (!c)
        return;
    /* Blur parva 3x3 */
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            int i = (y * w + x) * 4;
            for (int k = 0; k < 3; k++) {
                float s = 0.0f;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++)
                        s += c->pixels[((y+dy)*w + x+dx)*4 + k];
                float blurred = s / 9.0f;
                t->pixels[i+k] = mixf(t->pixels[i+k], blurred, vis * 0.6f);
            }
        }
    }
    tabula_dele(c);
    /* Tonus terreus: desat + shift ad aurantium/ochram */
    int n = w * h;
    for (int i = 0; i < n; i++) {
        float r = t->pixels[i*4+0];
        float g = t->pixels[i*4+1];
        float b = t->pixels[i*4+2];
        float lum = 0.299f * r + 0.587f * g + 0.114f * b;
        /* Mix cum ochra (0.72, 0.55, 0.30) proportionate ad lum */
        float tr = mixf(0.25f, 0.78f, lum);
        float tg = mixf(0.18f, 0.60f, lum);
        float tb = mixf(0.12f, 0.32f, lum);
        t->pixels[i*4+0] = mixf(r, tr, vis * 0.45f);
        t->pixels[i*4+1] = mixf(g, tg, vis * 0.45f);
        t->pixels[i*4+2] = mixf(b, tb, vis * 0.45f);
    }
}

/* FX_AURUM: folium aureum super partes lucidissimas (l > 0.75) */
static void fx_aurum(Tabula* t, float vis) {
    int n = t->w * t->h;
    for (int i = 0; i < n; i++) {
        float r = t->pixels[i*4+0];
        float g = t->pixels[i*4+1];
        float b = t->pixels[i*4+2];
        float l = 0.299f * r + 0.587f * g + 0.114f * b;
        float gold_mask = smoothstepf(0.70f, 0.92f, l);
        if (gold_mask < 0.001f)
            continue;
        /* Aurum: R=0.95, G=0.78, B=0.20; cum subtlě shimmer ex l ipso */
        float ar = 0.95f * (0.85f + 0.15f * l);
        float ag = 0.78f * (0.80f + 0.20f * l);
        float ab = 0.22f * l;
        float mix = gold_mask * vis;
        t->pixels[i*4+0] = mixf(r, ar, mix);
        t->pixels[i*4+1] = mixf(g, ag, mix);
        t->pixels[i*4+2] = mixf(b, ab, mix);
    }
}

/* FX_MOSAICUM: tesserae quadrata cum lineis commissuralibus obscuris */
static void fx_mosaicum(Tabula* t, float vis) {
    int w = t->w, h = t->h;
    Tabula* c = tabula_clona(t);
    if (!c)
        return;
    int cell = 4;
    for (int y = 0; y < h; y += cell) {
        for (int x = 0; x < w; x += cell) {
            /* Color medius cellae */
            float sr = 0, sg = 0, sb = 0;
            int cnt = 0;
            for (int dy = 0; dy < cell && y+dy < h; dy++) {
                for (int dx = 0; dx < cell && x+dx < w; dx++) {
                    int i = ((y+dy)*w + x+dx) * 4;
                    sr += c->pixels[i+0];
                    sg += c->pixels[i+1];
                    sb += c->pixels[i+2];
                    cnt++;
                }
            }
            float inv = 1.0f / (float)cnt;
            sr *= inv;
            sg *= inv;
            sb *= inv;
            /* Pingit cellam cum linea commissurali obscura in margine */
            for (int dy = 0; dy < cell && y+dy < h; dy++) {
                for (int dx = 0; dx < cell && x+dx < w; dx++) {
                    int i = ((y+dy)*w + x+dx) * 4;
                    int is_border = (dx == 0 || dy == 0);
                    float mult = is_border ? 0.55f : 1.0f;
                    float nr = sr * mult;
                    float ng = sg * mult;
                    float nb = sb * mult;
                    t->pixels[i+0] = mixf(t->pixels[i+0], nr, vis);
                    t->pixels[i+1] = mixf(t->pixels[i+1], ng, vis);
                    t->pixels[i+2] = mixf(t->pixels[i+2], nb, vis);
                }
            }
        }
    }
    tabula_dele(c);
}

/* FX_SOLARIZATIO: inversio tonalis super limine */
static void fx_solarizatio(Tabula* t, float vis) {
    int n = t->w * t->h;
    float thr = 0.55f;
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < 3; k++) {
            float v = t->pixels[i*4+k];
            if (v > thr) {
                float inv = 1.0f - (v - thr) * (1.0f / (1.0f - thr));
                t->pixels[i*4+k] = mixf(v, inv, vis);
            }
        }
    }
}

/* FX_RIMAE: rete rimarum mirariarum super imaginem */
static void fx_rimae(Tabula* t, float vis) {
    int w = t->w, h = t->h;
    uint32_t semen = 0xC9ACu;
    /* Noise fieldus ad detegendas "rimas" ubi gradient est abruptus */
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            float n00 = fbm_s((float)(x-1) * 0.18f, (float)y * 0.18f, 3, 0.5f, semen);
            float n10 = fbm_s((float)(x+1) * 0.18f, (float)y * 0.18f, 3, 0.5f, semen);
            float n01 = fbm_s((float)x * 0.18f, (float)(y-1) * 0.18f, 3, 0.5f, semen);
            float n11 = fbm_s((float)x * 0.18f, (float)(y+1) * 0.18f, 3, 0.5f, semen);
            float gx = fabsf(n10 - n00);
            float gy = fabsf(n11 - n01);
            float mag = gx + gy;
            /* Rima: thin dark line ubi magnitudo est prope limen */
            if (mag > 0.45f && mag < 0.60f) {
                float strength = (0.60f - fabsf(mag - 0.525f) * 10.0f) * vis;
                if (strength < 0.0f)
                    continue;
                int i = (y * w + x) * 4;
                t->pixels[i+0] *= (1.0f - strength * 0.7f);
                t->pixels[i+1] *= (1.0f - strength * 0.7f);
                t->pixels[i+2] *= (1.0f - strength * 0.7f);
            }
        }
    }
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
    case FX_PATINA:           fx_patina(t, vis); break;
    case FX_FRESCO:           fx_fresco(t, vis); break;
    case FX_AURUM:            fx_aurum(t, vis); break;
    case FX_MOSAICUM:         fx_mosaicum(t, vis); break;
    case FX_SOLARIZATIO:      fx_solarizatio(t, vis); break;
    case FX_RIMAE:            fx_rimae(t, vis); break;
    default: break;
    }
}

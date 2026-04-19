#include "modus.h"
#include "color.h"
#include "tumultus.h"
#include "commonia.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* COMICUS: 3-tone posterization + outline                            */
/* ------------------------------------------------------------------ */

static void modus_comicus(Tabula* t) {
    int w = t->w, h = t->h;
    /* Posterizatio: 3-4 gradus */
    for (int i = 0; i < w * h; i++) {
        Color c = color4(t->pixels[i*4+0], t->pixels[i*4+1], t->pixels[i*4+2], t->pixels[i*4+3]);
        Color p = color_posterizata(c, 4);
        t->pixels[i*4+0] = p.r;
        t->pixels[i*4+1] = p.g;
        t->pixels[i*4+2] = p.b;
    }
    /* Lineae marginum: Sobel-like detection, superimpositae */
    float* gray = (float*) malloc((size_t)w * h * sizeof(float));
    for (int i = 0; i < w * h; i++)
        gray[i] = color_luminantia(color4(t->pixels[i*4+0], t->pixels[i*4+1], t->pixels[i*4+2], 1));
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            float gx = gray[(y-1)*w + x + 1] - gray[(y-1)*w + x - 1]
                + 2.0f * (gray[y*w + x + 1] - gray[y*w + x - 1])
                + gray[(y+1)*w + x + 1] - gray[(y+1)*w + x - 1];
            float gy = gray[(y+1)*w + x - 1] - gray[(y-1)*w + x - 1]
                + 2.0f * (gray[(y+1)*w + x] - gray[(y-1)*w + x])
                + gray[(y+1)*w + x + 1] - gray[(y-1)*w + x + 1];
            float mag = sqrtf(gx*gx + gy*gy);
            if (mag > 0.15f) {
                float inten = saturatef((mag - 0.15f) * 2.5f);
                int i = (y*w + x) * 4;
                t->pixels[i+0] *= 1.0f - inten * 0.85f;
                t->pixels[i+1] *= 1.0f - inten * 0.85f;
                t->pixels[i+2] *= 1.0f - inten * 0.85f;
            }
        }
    }
    free(gray);
}

/* ------------------------------------------------------------------ */
/* TESSELLATUS: more musivi Romani — quadrata reducta + palette parva */
/* ------------------------------------------------------------------ */

static void modus_tessellatus(Tabula* t) {
    int w = t->w, h = t->h;
    int pixsize = 4;  /* 32x32 → 128 */
    int pw = w / pixsize;
    int ph = h / pixsize;
    /* Downsample: average each pixsize×pixsize block */
    float* avg = (float*) calloc((size_t)pw * ph * 4u, sizeof(float));
    for (int y = 0; y < ph; y++) {
        for (int x = 0; x < pw; x++) {
            float r = 0, g = 0, b = 0;
            for (int dy = 0; dy < pixsize; dy++) {
                for (int dx = 0; dx < pixsize; dx++) {
                    int ix = x * pixsize + dx;
                    int iy = y * pixsize + dy;
                    int i = (iy * w + ix) * 4;
                    r += t->pixels[i+0];
                    g += t->pixels[i+1];
                    b += t->pixels[i+2];
                }
            }
            float inv = 1.0f / (float)(pixsize * pixsize);
            int j = (y * pw + x) * 4;
            Color c = color4(r * inv, g * inv, b * inv, 1.0f);
            Color q = color_ad_palettam(c, PALETTA_PICO8, 16);
            avg[j+0] = q.r;
            avg[j+1] = q.g;
            avg[j+2] = q.b;
            avg[j+3] = 1.0f;
        }
    }
    /* Upsample back: nearest neighbor */
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int sx = x / pixsize;
            int sy = y / pixsize;
            if (sx >= pw)
                sx = pw - 1;
            if (sy >= ph)
                sy = ph - 1;
            int j = (sy * pw + sx) * 4;
            int i = (y * w + x) * 4;
            t->pixels[i+0] = avg[j+0];
            t->pixels[i+1] = avg[j+1];
            t->pixels[i+2] = avg[j+2];
        }
    }
    free(avg);
}

/* ------------------------------------------------------------------ */
/* ATRAMENTUM: monochrome + hatching                                  */
/* ------------------------------------------------------------------ */

static void modus_atramentum(Tabula* t) {
    int w = t->w, h = t->h;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int i = (y * w + x) * 4;
            Color c = color4(t->pixels[i+0], t->pixels[i+1], t->pixels[i+2], 1.0f);
            float lum = color_luminantia(c);

            /* Hatch density inversa ad luminantia */
            float rx = (float)x;
            float ry = (float)y;
            float h1 = sinf((rx + ry) * 0.9f);        /* hatch 45° */
            float h2 = sinf((rx - ry) * 0.9f);        /* hatch -45° */

            float hatch = 1.0f;
            if (lum < 0.28f) {
                /* duplici cruce */
                if (h1 > 0.0f || h2 > 0.0f)
                    hatch = 0.0f;
                else
                    hatch = 1.0f;
            } else if (lum < 0.50f) {
                if (h1 > 0.0f)
                    hatch = 0.0f;
                else
                    hatch = 1.0f;
            } else if (lum < 0.75f) {
                if (h1 > 0.5f)
                    hatch = 0.0f;
                else
                    hatch = 1.0f;
            } else {
                hatch = 1.0f;
            }
            /* Textura chartae */
            float paper = fbm_s(rx * 0.2f, ry * 0.2f, 3, 0.5f, 4242u) * 0.05f;
            hatch = clampf(hatch + paper, 0.0f, 1.0f);
            t->pixels[i+0] = 0.95f * hatch + 0.05f;
            t->pixels[i+1] = 0.92f * hatch + 0.05f;
            t->pixels[i+2] = 0.88f * hatch + 0.05f;
        }
    }
}

/* ------------------------------------------------------------------ */
/* PICTUM: painterly — directional blur + texture                     */
/* ------------------------------------------------------------------ */

static void modus_pictum(Tabula* t) {
    int w = t->w, h = t->h;
    Tabula* tmp = tabula_clona(t);
    if (!tmp)
        return;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            vec2 dir = fluxus_directio((float)x * 0.1f, (float)y * 0.1f, 1234u);
            int n = 5;
            float r = 0, g = 0, b = 0;
            int cnt = 0;
            for (int k = -n; k <= n; k++) {
                int sx = clampi(x + (int)(dir.x * (float)k), 0, w - 1);
                int sy = clampi(y + (int)(dir.y * (float)k), 0, h - 1);
                int i = (sy * w + sx) * 4;
                r += tmp->pixels[i+0];
                g += tmp->pixels[i+1];
                b += tmp->pixels[i+2];
                cnt++;
            }
            float inv = 1.0f / (float)cnt;
            int i = (y * w + x) * 4;
            t->pixels[i+0] = r * inv;
            t->pixels[i+1] = g * inv;
            t->pixels[i+2] = b * inv;
        }
    }
    /* Textura ictuum — strepitus cum color */
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float n = fbm_s((float)x * 0.3f, (float)y * 0.3f, 3, 0.5f, 9999u) * 0.10f;
            int i = (y * w + x) * 4;
            t->pixels[i+0] = clampf(t->pixels[i+0] + n, 0, 1);
            t->pixels[i+1] = clampf(t->pixels[i+1] + n * 0.8f, 0, 1);
            t->pixels[i+2] = clampf(t->pixels[i+2] + n * 0.6f, 0, 1);
        }
    }
    tabula_dele(tmp);
}

/* ------------------------------------------------------------------ */
/* LUDICRUM VIII: strong palette quantize + Bayer dither + scanlines  */
/* ------------------------------------------------------------------ */

static const float bayer4[16] = {
    0,  8,  2, 10,
    12,  4, 14,  6,
    3, 11,  1,  9,
    15,  7, 13,  5,
};

static void modus_ludicrum_viii(Tabula* t) {
    int w = t->w, h = t->h;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int i = (y * w + x) * 4;
            float bay = bayer4[(y % 4) * 4 + (x % 4)] / 16.0f - 0.5f;
            Color c = color4(
                t->pixels[i+0] + bay * 0.08f,
                t->pixels[i+1] + bay * 0.08f,
                t->pixels[i+2] + bay * 0.08f, 1.0f
            );
            Color q = color_ad_palettam(c, PALETTA_NES54, 54);
            t->pixels[i+0] = q.r;
            t->pixels[i+1] = q.g;
            t->pixels[i+2] = q.b;
        }
    }
    /* Scanlineae CRT */
    for (int y = 0; y < h; y += 2) {
        for (int x = 0; x < w; x++) {
            int i = (y * w + x) * 4;
            t->pixels[i+0] *= 0.85f;
            t->pixels[i+1] *= 0.85f;
            t->pixels[i+2] *= 0.85f;
        }
    }
}

/* ------------------------------------------------------------------ */
/* ORIENTALIS: stylus Orientis — colores saturati, cellae nitidae     */
/* ------------------------------------------------------------------ */

static void modus_orientalis(Tabula* t) {
    int w = t->w, h = t->h;
    for (int i = 0; i < w * h; i++) {
        Color c = color4(t->pixels[i*4+0], t->pixels[i*4+1], t->pixels[i*4+2], 1);
        HSL hsl = color_ad_hsl(c);
        hsl.s = saturatef(hsl.s * 1.25f + 0.05f);
        hsl.l = saturatef(hsl.l * 0.95f + 0.05f);
        Color o = hsl_ad_color(hsl, 1.0f);
        /* posterize lenis */
        o = color_posterizata(o, 6);
        t->pixels[i*4+0] = o.r;
        t->pixels[i*4+1] = o.g;
        t->pixels[i*4+2] = o.b;
    }
}

/* ------------------------------------------------------------------ */
/* NIGER: pure black and white + halftone mid-tones                   */
/* ------------------------------------------------------------------ */

static void modus_niger(Tabula* t) {
    int w = t->w, h = t->h;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int i = (y * w + x) * 4;
            Color c = color4(t->pixels[i+0], t->pixels[i+1], t->pixels[i+2], 1);
            float lum = color_luminantia(c);
            /* halftone medium */
            float cx = (float)((x % 4) - 2) + 0.5f;
            float cy = (float)((y % 4) - 2) + 0.5f;
            float d = sqrtf(cx*cx + cy*cy);
            float r_dot = 2.0f * (1.0f - lum);
            float v;
            if (lum > 0.85f)
                v = 1.0f;
            else if (lum < 0.15f)
                v = 0.0f;
            else
                v = d > r_dot ? 1.0f : 0.0f;
            t->pixels[i+0] = v;
            t->pixels[i+1] = v;
            t->pixels[i+2] = v;
        }
    }
}

void modus_applica(Tabula* t, ModusArtis modus, float tempus) {
    (void)tempus;
    switch (modus) {
    case MODUS_COMICUS:      modus_comicus(t);      break;
    case MODUS_TESSELLATUS:  modus_tessellatus(t);  break;
    case MODUS_ATRAMENTUM:   modus_atramentum(t);   break;
    case MODUS_PICTUM:       modus_pictum(t);       break;
    case MODUS_LUDICRUM_VIII:modus_ludicrum_viii(t);break;
    case MODUS_ORIENTALIS:   modus_orientalis(t);   break;
    case MODUS_NIGER:        modus_niger(t);        break;
    default: break;
    }
}

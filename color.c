#include "color.h"

HSL color_ad_hsl(Color c) {
    float mx = fmaxf(c.r, fmaxf(c.g, c.b));
    float mn = fminf(c.r, fminf(c.g, c.b));
    HSL h = {0, 0, (mx + mn) * 0.5f};
    if (mx == mn) {
        h.h = 0.0f;
        h.s = 0.0f;
        return h;
    }
    float d = mx - mn;
    h.s = h.l > 0.5f ? d / (2.0f - mx - mn) : d / (mx + mn);
    if      (mx == c.r)
        h.h = (c.g - c.b) / d + (c.g < c.b ? 6.0f : 0.0f);
    else if (mx == c.g)
        h.h = (c.b - c.r) / d + 2.0f;
    else
        h.h = (c.r - c.g) / d + 4.0f;
    h.h /= 6.0f;
    return h;
}

static float hue_ad_rgb(float p, float q, float t) {
    if (t < 0.0f)
        t += 1.0f;
    if (t > 1.0f)
        t -= 1.0f;
    if (t < 1.0f/6.0f)
        return p + (q - p) * 6.0f * t;
    if (t < 1.0f/2.0f)
        return q;
    if (t < 2.0f/3.0f)
        return p + (q - p) * (2.0f/3.0f - t) * 6.0f;
    return p;
}

Color hsl_ad_color(HSL hsl, float a) {
    float r, g, b;
    if (hsl.s == 0.0f) {
        r = g = b = hsl.l;
    } else {
        float q = hsl.l < 0.5f ? hsl.l * (1.0f + hsl.s) : hsl.l + hsl.s - hsl.l * hsl.s;
        float p = 2.0f * hsl.l - q;
        r = hue_ad_rgb(p, q, hsl.h + 1.0f/3.0f);
        g = hue_ad_rgb(p, q, hsl.h);
        b = hue_ad_rgb(p, q, hsl.h - 1.0f/3.0f);
    }
    return color4(r, g, b, a);
}

/* Formula cutis: interpolatio inter colores "clarissimus" et "obscurissimus"
 * cum modulatione temperaturae per hue. */
Color color_cutis_basis(float color_cutis, float calor_cutis) {
    color_cutis = saturatef(color_cutis);
    calor_cutis = saturatef(calor_cutis);

    /* Hua: spectrum latius ab roseo-frigido (0.98, circa wrap) ad aurantio-ochreum (0.10).
     * calor_cutis < 0.3 = frigidus (roseus porcellaneus), 0.3-0.7 = neutralis peachy,
     * > 0.7 = calidus olivaceus/ochreus.  Cutes obscuriores shift ad warmer ochre. */
    float h_cool = 0.99f;   /* roseus porcellaneus (wraps near 0) */
    float h_neut = 0.05f;   /* aurantius-roseus */
    float h_warm = 0.10f;   /* ochra/olive */
    float h;
    if (calor_cutis < 0.35f) {
        float u = calor_cutis / 0.35f;
        /* Wrap-aware interpolation ab 0.99 ad 0.05 per iter breve (per 0/1) */
        h = (1.0f - u) * h_cool + u * (h_neut + 1.0f);
        if (h >= 1.0f)
            h -= 1.0f;
    } else {
        float u = (calor_cutis - 0.35f) / 0.65f;
        h = mixf(h_neut, h_warm, u);
    }
    /* Cutes obscuriores (color_cutis alta) magis ochreae/fulvae */
    h = mixf(h, mixf(h, 0.09f, 0.6f), color_cutis * 0.55f);

    /* Saturatio: pallidi desaturati, obscuri ricchi */
    float s = mixf(0.22f, 0.68f, color_cutis);
    /* Lightness: ab pallidissimo ad profundum — curva non-lineara ut medius lūcidior sit */
    float l = mixf(0.90f, 0.18f, color_cutis * color_cutis * 0.5f + color_cutis * 0.5f);

    HSL hsl = { h, s, l };
    return hsl_ad_color(hsl, 1.0f);
}

Color color_iridis_base(float h, float s, float l) {
    HSL hsl = { saturatef(h), saturatef(s), saturatef(l) };
    return hsl_ad_color(hsl, 1.0f);
}

Color color_comae(float h, float s, float v, float canitudo) {
    /* "v" hic tractatur ut lightness */
    HSL hsl = { saturatef(h), saturatef(s) * (1.0f - canitudo), mixf(saturatef(v), 0.85f, canitudo) };
    return hsl_ad_color(hsl, 1.0f);
}

Color color_labiorum(Color cutis, float saturatio) {
    /* Hue est circularis: interpolatio liniaris inter 0.27 (viride) et 0.98
     * (rubrum magicum) producit purpuream non-intenditam. Adhibemus iter
     * brevius in circulo. */
    HSL h = color_ad_hsl(cutis);
    float target = 0.00f;           /* rubrum */
    float dh = target - h.h;
    if (dh >  0.5f)
        dh -= 1.0f;
    if (dh < -0.5f)
        dh += 1.0f;
    h.h = h.h + dh * 0.50f;         /* semi-viam ad rubrum per iter brevius */
    if (h.h < 0.0f)
        h.h += 1.0f;
    if (h.h >= 1.0f)
        h.h -= 1.0f;
    h.s = clampf(h.s + 0.25f * saturatio, 0.0f, 1.0f);
    h.l = clampf(h.l - 0.12f, 0.0f, 1.0f);
    return hsl_ad_color(h, 1.0f);
}

Color color_sclera(float vena_pondus) {
    /* leviter cremeus in aetate / vena */
    float base = 0.96f - 0.15f * saturatef(vena_pondus);
    return color4(base, base - 0.02f, base - 0.05f, 1.0f);
}

Color color_misce(Color a, Color b, float t) {
    t = saturatef(t);
    return color4(mixf(a.r, b.r, t), mixf(a.g, b.g, t), mixf(a.b, b.b, t), mixf(a.a, b.a, t));
}

Color color_desaturatus(Color c, float t) {
    float lum = color_luminantia(c);
    return color4(mixf(c.r, lum, t), mixf(c.g, lum, t), mixf(c.b, lum, t), c.a);
}

Color color_obscurior(Color c, float t) {
    t = saturatef(t);
    return color4(c.r * (1.0f - t), c.g * (1.0f - t), c.b * (1.0f - t), c.a);
}

Color color_clariorem(Color c, float t) {
    t = saturatef(t);
    return color4(mixf(c.r, 1.0f, t), mixf(c.g, 1.0f, t), mixf(c.b, 1.0f, t), c.a);
}

Color color_posterizata(Color c, int gradus) {
    if (gradus < 2)
        gradus = 2;
    float g = (float)(gradus - 1);
    return color4(
        floorf(c.r * g + 0.5f) / g,
        floorf(c.g * g + 0.5f) / g,
        floorf(c.b * g + 0.5f) / g,
        c.a
    );
}

Color color_cel_toni(Color basis, float lum, float t_umbra, float t_lux) {
    /* 3 toni discreti ex lum */
    Color umbra  = color_obscurior(basis, 0.40f);
    Color medius = basis;
    Color lux    = color_clariorem(basis, 0.25f);
    if (lum < t_umbra)
        return umbra;
    else if (lum > t_lux)
        return lux;
    else
        return medius;
}

/* Palettae */
#define C(r,g,b) { (r)/255.0f, (g)/255.0f, (b)/255.0f, 1.0f }

const Color PALETTA_CGA16[16] = {
    C(0, 0, 0),       C(0, 0, 170),    C(0, 170, 0),    C(0, 170, 170),
    C(170, 0, 0),     C(170, 0, 170),  C(170, 85, 0),   C(170, 170, 170),
    C(85, 85, 85),    C(85, 85, 255),  C(85, 255, 85),  C(85, 255, 255),
    C(255, 85, 85),   C(255, 85, 255), C(255, 255, 85), C(255, 255, 255),
};

const Color PALETTA_PICO8[16] = {
    C(0, 0, 0),        C(29, 43, 83),    C(126, 37, 83),   C(0, 135, 81),
    C(171, 82, 54),    C(95, 87, 79),    C(194, 195, 199), C(255, 241, 232),
    C(255, 0, 77),     C(255, 163, 0),   C(255, 236, 39),  C(0, 228, 54),
    C(41, 173, 255),   C(131, 118, 156), C(255, 119, 168), C(255, 204, 170),
};

/* NES 54 — subset colorum palettae NES (52 + duo blancae) */
const Color PALETTA_NES54[54] = {
    C(124, 124, 124), C(0, 0, 252),     C(0, 0, 188),     C(68, 40, 188),
    C(148, 0, 132),   C(168, 0, 32),    C(168, 16, 0),    C(136, 20, 0),
    C(80, 48, 0),     C(0, 120, 0),     C(0, 104, 0),     C(0, 88, 0),
    C(0, 64, 88),     C(0, 0, 0),       C(0, 0, 0),       C(0, 0, 0),
    C(188, 188, 188), C(0, 120, 248),   C(0, 88, 248),    C(104, 68, 252),
    C(216, 0, 204),   C(228, 0, 88),    C(248, 56, 0),    C(228, 92, 16),
    C(172, 124, 0),   C(0, 184, 0),     C(0, 168, 0),     C(0, 168, 68),
    C(0, 136, 136),   C(0, 0, 0),       C(0, 0, 0),       C(0, 0, 0),
    C(248, 248, 248), C(60, 188, 252),  C(104, 136, 252), C(152, 120, 248),
    C(248, 120, 248), C(248, 88, 152),  C(248, 120, 88),  C(252, 160, 68),
    C(248, 184, 0),   C(184, 248, 24),  C(88, 216, 84),   C(88, 248, 152),
    C(0, 232, 216),   C(120, 120, 120), C(0, 0, 0),       C(0, 0, 0),
    C(252, 252, 252), C(164, 228, 252), C(184, 184, 248), C(216, 184, 248),
    C(248, 184, 248), C(248, 164, 192),
};

#undef C

Color color_ad_palettam(Color c, const Color* paletta, int n) {
    int best_i = 0;
    float best_d = 1e10f;
    for (int i = 0; i < n; i++) {
        float dr = c.r - paletta[i].r;
        float dg = c.g - paletta[i].g;
        float db = c.b - paletta[i].b;
        float d = dr*dr + dg*dg + db*db;
        if (d < best_d) {
            best_d = d;
            best_i = i;
        }
    }
    Color r = paletta[best_i];
    r.a = c.a;
    return r;
}

/* color.h — operationes coloris, mathematica cutis, palettae. */
#ifndef PORTRAIT_COLOR_H
#define PORTRAIT_COLOR_H

#include "commonia.h"
#include "tabula.h"    /* Color */

typedef struct {
    float h, s, l;
}HSL;

/* Conversiones */
HSL   color_ad_hsl(Color c);
Color hsl_ad_color(HSL h, float a);

/* Color cutis ex parametris (formula — non paletta) */
Color color_cutis_basis(float color_cutis, float calor_cutis);

/* Color irium ex h, s */
Color color_iridis_base(float h, float s, float l);

/* Color comae ex HSV-like parametra — adhibemus h, s, v cum canitudine */
Color color_comae(float h, float s, float v, float canitudo);

/* Color labiorum ex cute */
Color color_labiorum(Color cutis, float saturatio);

/* Color sclerae */
Color color_sclera(float vena_pondus);

/* Mixtio duorum colorum per t */
Color color_misce(Color a, Color b, float t);

/* Desaturatio pro canitie */
Color color_desaturatus(Color c, float t);

/* Darken / lighten */
Color color_obscurior(Color c, float t);
Color color_clariorem(Color c, float t);

/* Multiplicatio componentium */
static inline Color color_mul_f(Color c, float m) {
    return color4(c.r * m, c.g * m, c.b * m, c.a);
}

/* Luminantia (BT.709) */
static inline float color_luminantia(Color c) {
    return 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b;
}

/* Quantizatio ad N gradus (posterizatio) */
Color color_posterizata(Color c, int gradus);

/* Applica toni cartooni: 3 toni (umbra/medius/lux) ex valore input luminantia */
Color color_cel_toni(Color basis, float lum, float t_umbra, float t_lux);

/* Palettae parvae */
extern const Color PALETTA_CGA16[16];
extern const Color PALETTA_NES54[54];
extern const Color PALETTA_PICO8[16];

/* Quantizatio ad palettam — elegit colorem proximum euclidee */
Color color_ad_palettam(Color c, const Color* paletta, int n);

#endif

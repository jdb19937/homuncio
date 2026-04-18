/* tabula.h — capsa RGBA float, modi mixturae, anti-aliasing. */
#ifndef PORTRAIT_TABULA_H
#define PORTRAIT_TABULA_H

#include "commonia.h"

typedef struct {
    int   w, h;
    float* pixels;   /* w*h*4, ordo RGBA, praemultiplicatus linearis */
} Tabula;

typedef struct {
    float r, g, b, a;
}Color;

static inline Color color4(float r, float g, float b, float a) {
    Color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

/* Creatio/deletio */
Tabula* tabula_nova(int w, int h);
void    tabula_dele(Tabula* t);

/* Tabulam colore implet */
void    tabula_imple(Tabula* t, Color c);

/* Alpha-mixtio pixelem unum (non-praemultiplicata) */
void    tabula_misce(Tabula* t, int x, int y, Color c);

/* Directa positio pixelem sine mixtura */
void    tabula_pone(Tabula* t, int x, int y, Color c);

/* Lecturam: sicut Color, fuera fines: Color(0,0,0,0) */
Color   tabula_lege(const Tabula* t, int x, int y);

/* Inversum: tabula uno colore supra, miscendo per amplitudinem */
void    tabula_misce_pondus(Tabula* t, int x, int y, Color c, float pondus);

/* Pinge rectangulum coopertum per SDF-probram (d < 0 ⇒ intus), cum AA lenis */
typedef float (*SdfProba)(vec2 p, void* ctx);
void tabula_pinge_sdf(
    Tabula* t, vec2 centrum, float raggio, SdfProba f,
    void* ctx, Color c
);

/* Pinge lineam inter duas puncta (AA per SDF capsulae) */
void tabula_pinge_lineam(Tabula* t, vec2 a, vec2 b, float crassitudo, Color c);

/* Pinge discum */
void tabula_pinge_discum(Tabula* t, vec2 centrum, float r, Color c);

/* Conversio ex fluxu ad RGBA8 (clamp et gamma 2.2 → sRGB approximatus) */
void tabula_ad_rgba8(const Tabula* t, uint8_t* out);

/* Clone (allocatio nova) */
Tabula* tabula_clona(const Tabula* t);

/* Copia pixel per pixel */
void tabula_copia(Tabula* dst, const Tabula* src);

#endif

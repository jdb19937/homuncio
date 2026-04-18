/* imago.c — struct Imago + pipelinae publicae. */
#include "homuncio.h"
#include "commonia.h"
#include "tabula.h"
#include "facies.h"
#include "archetypum.h"
#include "expressio.h"
#include "lineamenta.h"
#include "coma.h"
#include "cutis.h"
#include "color.h"
#include "modus.h"
#include "effectus.h"
#include "vestis.h"
#include "ornamenta.h"
#include "exitus.h"
#include "sors.h"
#include "tumultus.h"
#include <stdlib.h>
#include <string.h>

struct Imago {
    FaciesParametra par;
    Expressio       expressio_active;
    float           expressio_pondus;
};

/* ---------------------------------------------------------------------------
 * Constructor / destructor
 * ------------------------------------------------------------------------ */

Imago* imago_nova(uint64_t semen, Archetypum arch, Gens gens) {
    Imago* im = (Imago*) calloc(1, sizeof(Imago));
    if (!im)
        return NULL;
    facies_genera(&im->par, semen);
    archetypum_applica(&im->par, arch);
    gens_applica(&im->par, gens);
    im->expressio_active = EXPR_NEUTRUM;
    im->expressio_pondus = 0.0f;
    return im;
}

void imago_dele(Imago* im) { free(im); }

void imago_pone_parametrum(Imago* im, const char* clavis, float valor) {
    if (!im)
        return;
    (void) facies_pone_clavem(&im->par, clavis, valor);
}

void imago_pone_expressionem(Imago* im, Expressio expr, float pondus) {
    if (!im)
        return;
    im->expressio_active = expr;
    im->expressio_pondus = pondus;
}

/* ---------------------------------------------------------------------------
 * Sfondo — gradient simplex
 * ------------------------------------------------------------------------ */

static void redde_sfondo(Tabula* t, const FaciesParametra* p) {
    int w = t->w, h = t->h;
    Sors s = sors_deriva(p->semen, 0xBACBACu);

    /* Duo stili: ~60% subtilis frigidus, ~40% audax primarius vivido saturatione */
    int audax = sors_f32(&s) < 0.40f;
    float hue, sat_t, sat_b, lum_t, lum_b;

    if (audax) {
        /* Paletta primaria — hua ex cumulo colorum pop */
        float roll = sors_f32(&s);
        if      (roll < 0.16f)
            hue = sors_spatium(&s, 0.00f, 0.04f);  /* ruber */
        else if (roll < 0.32f)
            hue = sors_spatium(&s, 0.06f, 0.10f);  /* aurantius */
        else if (roll < 0.44f)
            hue = sors_spatium(&s, 0.12f, 0.16f);  /* flavus */
        else if (roll < 0.58f)
            hue = sors_spatium(&s, 0.26f, 0.38f);  /* viridis */
        else if (roll < 0.72f)
            hue = sors_spatium(&s, 0.50f, 0.58f);  /* cyaneus */
        else if (roll < 0.86f)
            hue = sors_spatium(&s, 0.60f, 0.68f);  /* caeruleus saturatus */
        else
            hue = sors_spatium(&s, 0.83f, 0.95f);  /* purpureus-roseus */
        sat_t = sors_spatium(&s, 0.65f, 0.88f);
        sat_b = sors_spatium(&s, 0.70f, 0.90f);
        lum_t = sors_spatium(&s, 0.35f, 0.55f);
        lum_b = sors_spatium(&s, 0.20f, 0.38f);
    } else {
        /* Subtilis: hua frigida cum saturatione humili */
        hue = sors_f32(&s) < 0.5f
            ? sors_spatium(&s, 0.55f, 0.70f)
            : sors_spatium(&s, 0.72f, 0.82f);
        sat_t = sors_spatium(&s, 0.12f, 0.25f);
        sat_b = sors_spatium(&s, 0.10f, 0.22f);
        lum_t = sors_spatium(&s, 0.18f, 0.30f);
        lum_b = sors_spatium(&s, 0.08f, 0.15f);
    }

    HSL h1 = { hue, sat_t, lum_t };
    HSL h2 = { hue, sat_b, lum_b };
    Color top = hsl_ad_color(h1, 1.0f);
    Color bot = hsl_ad_color(h2, 1.0f);
    for (int y = 0; y < h; y++) {
        float u = (float)y / (float)(h - 1);
        Color c = color_misce(top, bot, u);
        for (int x = 0; x < w; x++) {
            int i = (y * w + x) * 4;
            t->pixels[i+0] = c.r;
            t->pixels[i+1] = c.g;
            t->pixels[i+2] = c.b;
            t->pixels[i+3] = 1.0f;
        }
    }
}

/* Cervix + humeri — trapezium busti proprium. */
static void redde_cervix(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    (void)p;

    float cx  = z->centrum_faciei.x;
    float cy  = z->cervix.y_sup;
    float h_f = (float) t->h;
    float neck_half = z->lat_faciei * 0.28f;

    /* Humeri trapezium — pingimus ut seriem linearum verticaliterum */
    float sh_top_y = cy + z->alt_faciei * 0.12f;  /* ubi humeri incipiunt */
    float sh_half_top = z->lat_faciei * 0.55f;
    float sh_half_bot = z->lat_faciei * 1.20f;
    int Nshoulders = 20;
    for (int i = 0; i < Nshoulders; i++) {
        float u = (float)i / (float)(Nshoulders - 1);
        float y = mixf(sh_top_y, h_f + 2.0f, u);
        float half = mixf(sh_half_top, sh_half_bot, u);
        tabula_pinge_lineam(t, v2(cx - half, y), v2(cx + half, y), 1.8f, col->cutis);
    }

    /* Cervix — capsula verticalis ante humeros */
    int Nneck = 16;
    for (int i = 0; i < Nneck; i++) {
        float u = (float)i / (float)(Nneck - 1);
        float y = mixf(cy - 1.0f, sh_top_y + 2.0f, u);
        float half = mixf(neck_half, neck_half * 1.1f, u);
        tabula_pinge_lineam(t, v2(cx - half, y), v2(cx + half, y), 1.5f, col->cutis);
    }

    /* Umbra sub mento — gradient lenis */
    Color umb = col->cutis_umbra;
    umb.a = 0.45f;
    tabula_pinge_discum(t, v2(cx, cy + 1.0f), neck_half * 1.35f, umb);

    /* Umbra in lateribus cervicis */
    Color umb2 = col->cutis_umbra;
    umb2.a = 0.3f;
    tabula_pinge_lineam(t, v2(cx - neck_half, cy), v2(cx - neck_half, sh_top_y), 2.0f, umb2);
    tabula_pinge_lineam(t, v2(cx + neck_half, cy), v2(cx + neck_half, sh_top_y), 2.0f, umb2);
}

/* Applicat biolumiscentiam (nitor specialis) */
static void redde_bioluminescentia(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z) {
    if (p->bioluminescentia < 0.05f)
        return;
    float amp = p->bioluminescentia;
    float pulse = 0.7f + 0.3f * sinf(PORTRAIT_TAU * p->tempus);
    Color glow = color4(0.4f, 0.9f, 1.0f, 0.35f * amp * pulse);
    int w = t->w, h = t->h;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float d = sdf_contura_faciei(v2((float)x + 0.5f, (float)y + 0.5f), p, z);
            if (d > 1.0f || d < -8.0f)
                continue;
            float g = smoothstepf(1.0f, -4.0f, d);
            Color c = glow;
            c.a *= g;
            tabula_misce(t, x, y, c);
        }
    }
}

/* ---------------------------------------------------------------------------
 * Pipelina redditionis principalis
 * ------------------------------------------------------------------------ */

static void imago_redde_impl(
    Imago* im, uint8_t* rgba, ModusArtis modus,
    const PostEffectus* fxs, const float* vis_fxs, int n_fx,
    float tempus
) {
    if (!im || !rgba)
        return;

    /* Applicamus expressionem (non destructive — copia paramum) */
    FaciesParametra par = im->par;
    par.tempus = tempus;

    if (im->expressio_pondus > 0.001f) {
        facies_applica_expressionem(&par, &im->expressio_active, im->expressio_pondus);
    }

    /* Tremor lucis: color cutis oscillat leviter */
    float tremor = cosf(PORTRAIT_TAU * tempus) * 0.015f;
    par.calor_cutis = clampf(par.calor_cutis + tremor, 0.0f, 1.0f);

    /* Redditio */
    Tabula* t = tabula_nova(PORTRAIT_LATITUDO, PORTRAIT_ALTITUDO);
    if (!t)
        return;

    ZonaeFaciei z = zonae_computa(&par, t->w, t->h);
    PalettaFaciei col = paletta_computa(&par);

    /* Ordo:
     *   1. sfondo
     *   2. aures (utrum posterior ad faciem)
     *   3. cervix
     *   4. cutis basis
     *   5. texturae cutis (pori, subtilis)
     *   6. rugae
     *   7. lentigines
     *   8. cicatrices
     *   9. nasus
     *   10. os
     *   11. dentes
     *   12. supercilia
     *   13. oculi
     *   14. cornua
     *   15. barba, mustacia
     *   16. massa comae
     *   17. fila comae
     *   18. bioluminescentia (glow)
     */
    redde_sfondo(t, &par);
    redde_aures(t, &par, &z, &col);
    redde_cervix(t, &par, &z, &col);
    redde_cutis_base(t, &par, &z, &col);
    redde_cutis_textura(t, &par, &z, &col);
    redde_rugas(t, &par, &z, &col);
    redde_lentigines(t, &par, &z, &col);
    redde_cicatrices(t, &par, &z, &col);
    redde_nasum(t, &par, &z, &col);
    redde_os(t, &par, &z, &col);
    redde_dentes(t, &par, &z, &col);
    redde_supercilia(t, &par, &z, &col);
    redde_oculos(t, &par, &z, &col);
    redde_cornua(t, &par, &z, &col);
    redde_barba(t, &par, &z, &col);
    redde_mustacia(t, &par, &z, &col);
    redde_vestis(t, &par, &z, &col);
    redde_coma_massa(t, &par, &z, &col);
    redde_coma_fila(t, &par, &z, &col);
    redde_ornamenta_capitis(t, &par, &z, &col);
    redde_bioluminescentia(t, &par, &z);

    /* Applica stilum */
    modus_applica(t, modus, tempus);

    /* Applica post-effectus in sequentia */
    for (int k = 0; k < n_fx; k++)
        effectus_applica(t, fxs[k], vis_fxs[k], tempus);

    /* Conversio ad RGBA8 */
    tabula_ad_rgba8(t, rgba);
    tabula_dele(t);
}

void imago_redde(
    Imago* im, uint8_t* rgba, ModusArtis modus,
    PostEffectus fx, float vis_fx, float tempus
) {
    PostEffectus fxs[1] = { fx };
    float        vis[1] = { vis_fx };
    imago_redde_impl(im, rgba, modus, fxs, vis, 1, tempus);
}

void imago_redde_fx2(
    Imago* im, uint8_t* rgba, ModusArtis modus,
    PostEffectus fx1, float vis_fx1,
    PostEffectus fx2, float vis_fx2,
    float tempus
) {
    PostEffectus fxs[2] = { fx1, fx2 };
    float        vis[2] = { vis_fx1, vis_fx2 };
    int n = (fx2 == FX_NULLUS) ? 1 : 2;
    imago_redde_impl(im, rgba, modus, fxs, vis, n, tempus);
}

int imago_crea_ppm(
    uint64_t semen, Archetypum arch, Gens gens,
    ModusArtis modus, float tempus, const char* via
) {
    Imago* im = imago_nova(semen, arch, gens);
    if (!im)
        return -1;
    uint8_t* buf = (uint8_t*) malloc((size_t)PORTRAIT_LATITUDO * PORTRAIT_ALTITUDO * 4u);
    if (!buf) {
        imago_dele(im);
        return -1;
    }
    imago_redde(im, buf, modus, FX_NULLUS, 0.0f, tempus);
    int r = exitus_scribe_ppm(via, buf, PORTRAIT_LATITUDO, PORTRAIT_ALTITUDO);
    free(buf);
    imago_dele(im);
    return r;
}

/* ---------------------------------------------------------------------------
 * Dialog border
 * ------------------------------------------------------------------------ */

static void margo_pinge(Tabula* t, MargoModus m, float tempus) {
    if (m == MARGO_NULLUS)
        return;
    int w = t->w, h = t->h;
    int margo_w = 6;
    Color base;
    uint32_t semen;
    switch (m) {
    case MARGO_LAPIDIS: base = color4(0.48f, 0.48f, 0.48f, 1.0f);
        semen = 1u;
        break;
    case MARGO_LIGNI:   base = color4(0.45f, 0.27f, 0.13f, 1.0f);
        semen = 2u;
        break;
    case MARGO_ARCANUM: base = color4(0.25f, 0.15f, 0.45f, 1.0f);
        semen = 3u;
        break;
    case MARGO_FERRI:   base = color4(0.32f, 0.32f, 0.36f, 1.0f);
        semen = 4u;
        break;
    default: return;
    }

    /* Pulse for arcanum */
    float pulse = 1.0f;
    if (m == MARGO_ARCANUM) {
        pulse = 0.8f + 0.4f * sinf(PORTRAIT_TAU * tempus * 0.5f);
    }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int border_dist = x;
            if (w - 1 - x < border_dist)
                border_dist = w - 1 - x;
            if (y < border_dist)
                border_dist = y;
            if (h - 1 - y < border_dist)
                border_dist = h - 1 - y;
            if (border_dist >= margo_w)
                continue;
            float noise = fbm_s((float)x * 0.3f, (float)y * 0.3f, 3, 0.5f, semen) * 0.5f + 0.5f;
            Color c = base;
            c.r *= (0.7f + 0.6f * noise) * pulse;
            c.g *= (0.7f + 0.6f * noise) * pulse;
            c.b *= (0.7f + 0.6f * noise) * pulse;
            /* bevel: clariorem ad intus */
            float edge = 1.0f - (float)border_dist / (float)margo_w;
            c.r *= (0.55f + 0.45f * (1.0f - edge));
            c.g *= (0.55f + 0.45f * (1.0f - edge));
            c.b *= (0.55f + 0.45f * (1.0f - edge));
            int i = (y * w + x) * 4;
            t->pixels[i+0] = c.r;
            t->pixels[i+1] = c.g;
            t->pixels[i+2] = c.b;
        }
    }
}

void imago_margo_dialogi(
    Imago* im, uint8_t* rgba, ModusArtis modus,
    MargoModus margo, float tempus
) {
    if (!im || !rgba)
        return;
    /* Redde imaginem normaliter, deinde superpone margo in tabulem */
    /* Pro simplicitate, reddimus imaginem, conversimus, et imponimus margo post. */
    uint8_t* temp = (uint8_t*) malloc((size_t)PORTRAIT_LATITUDO * PORTRAIT_ALTITUDO * 4u);
    if (!temp)
        return;
    imago_redde(im, temp, modus, FX_NULLUS, 0.0f, tempus);

    /* Construam tabulam ex temp, pingam margo, reconvertam */
    Tabula* t = tabula_nova(PORTRAIT_LATITUDO, PORTRAIT_ALTITUDO);
    if (!t) {
        free(temp);
        return;
    }
    for (int i = 0; i < PORTRAIT_LATITUDO * PORTRAIT_ALTITUDO; i++) {
        t->pixels[i*4+0] = (float)temp[i*4+0] / 255.0f;
        t->pixels[i*4+1] = (float)temp[i*4+1] / 255.0f;
        t->pixels[i*4+2] = (float)temp[i*4+2] / 255.0f;
        t->pixels[i*4+3] = 1.0f;
    }
    /* Note: no sRGB inverse — acceptable approximatio */
    margo_pinge(t, margo, tempus);
    tabula_ad_rgba8(t, rgba);
    tabula_dele(t);
    free(temp);
}

/* ---------------------------------------------------------------------------
 * Textual name resolution (pro CLI)
 * ------------------------------------------------------------------------ */

static int find_name(const char* s, const char* const* tab, int n) {
    for (int i = 0; i < n; i++) {
        if (strcmp(s, tab[i]) == 0)
            return i;
    }
    return -1;
}

int portrait_archetypum_ex_nomine(const char* nomen) {
    static const char* const tab[] = {
        "lar", "manes", "lemur", "ianus", "augur",
        "quirinus", "carna", "genius", "flamen", "faunus"
    };
    return find_name(nomen, tab, 10);
}

int portrait_gens_ex_nomine(const char* nomen) {
    static const char* const tab[] = {
        "humana", "nympha", "nanus", "gigas",
        "penates", "larva", "furia", "satyrus"
    };
    return find_name(nomen, tab, 8);
}

int portrait_modus_ex_nomine(const char* nomen) {
    static const char* const tab[] = {
        "cartoon", "pixel", "atramentum", "pictum",
        "ludicrum_viii", "anime", "niger"
    };
    return find_name(nomen, tab, 7);
}

int portrait_effectus_ex_nomine(const char* nomen) {
    static const char* const tab[] = {
        "nullus", "vignetta", "granum", "scanlineae_crt",
        "aber_chromatis", "dithering_bayer", "dithering_fs",
        "halftone", "posterizatio", "lineae_prominentes", "nitor"
    };
    return find_name(nomen, tab, 11);
}

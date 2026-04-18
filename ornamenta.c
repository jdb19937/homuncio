#include "ornamenta.h"
#include "color.h"
#include "sdf.h"
#include "sors.h"

/* --- Corona laurea: arcus foliorum lauri super frontem et cranium --- */

static void corona_laurea(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z) {
    int n = 14;
    Color leaf_base = color4(0.20f, 0.45f, 0.18f, 1.0f);   /* viridis olivaceus */
    Color leaf_dark = color4(0.10f, 0.25f, 0.10f, 1.0f);
    Color leaf_light = color4(0.35f, 0.60f, 0.25f, 1.0f);

    Sors s = sors_deriva(p->semen, 0xC0A0A0u);

    /* Arcus: ab latere sinistro caput supra ad latus dextrum */
    float y_center = z->frons.y_culmen - z->alt_faciei * 0.02f;
    float rx = z->lat_faciei * 1.00f;
    float ry = z->alt_faciei * 0.55f;

    for (int i = 0; i < n; i++) {
        float u = (float)i / (float)(n - 1);
        /* Angulus ab ~195 deg ad ~-15 deg (arcus supra) */
        float ang = mixf(PORTRAIT_PI * 1.05f, -PORTRAIT_PI * 0.05f, u);
        float cx = z->centrum_faciei.x + cosf(ang) * rx;
        float cy = y_center + sinf(ang) * ry * 0.80f;

        /* Folium: lens orientatum tangentialiter arcui */
        float tx = -sinf(ang);
        float ty = cosf(ang);
        float leaf_len = 4.5f + sors_f32(&s) * 2.0f;
        vec2 a = v2(cx - tx * leaf_len, cy - ty * leaf_len);
        vec2 b = v2(cx + tx * leaf_len, cy + ty * leaf_len);

        Color c;
        float r = sors_f32(&s);
        if (r < 0.33f)
            c = leaf_dark;
        else if (r < 0.66f)
            c = leaf_base;
        else
            c = leaf_light;
        tabula_pinge_lineam(t, a, b, 2.4f, c);
    }

    /* Bacca interdum — parva rubra inter folia */
    if (sors_f32(&s) < 0.4f) {
        Color berry = color4(0.65f, 0.15f, 0.15f, 1.0f);
        for (int i = 0; i < 3; i++) {
            float ang = sors_spatium(&s, PORTRAIT_PI * 0.8f, PORTRAIT_PI * 0.2f);
            float bx = z->centrum_faciei.x + cosf(ang) * rx * 0.95f;
            float by = y_center + sinf(ang) * ry * 0.75f;
            tabula_pinge_discum(t, v2(bx, by), 1.2f, berry);
        }
    }
}

/* --- Galea: dome + crista equina + paragnathides --- */

typedef struct {
    vec2 c;
    float rx, ry;
    float y_cut;
}GaleaCtx;
static float sdf_galea_dome(vec2 p, void* ctx_) {
    GaleaCtx* ctx = (GaleaCtx*) ctx_;
    vec2 q = v2_sub(p, ctx->c);
    float d_ell = sdf_ellipsis(q, ctx->rx, ctx->ry);
    /* Secamus dimidium inferius (tantum summa pars galea) */
    float d_cut = p.y - ctx->y_cut;
    return sdf_intersectio(d_ell, d_cut);
}

static void galea(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z) {
    (void)p;
    /* Dome */
    GaleaCtx ctx;
    ctx.c.x = z->centrum_faciei.x;
    ctx.c.y = z->frons.y_culmen + z->alt_faciei * 0.08f;
    ctx.rx = z->lat_faciei * 1.18f;
    ctx.ry = z->alt_faciei * 0.72f;
    ctx.y_cut = z->frons.y_frons + z->alt_faciei * 0.04f;  /* just above eyebrows */

    Color bronze      = color4(0.55f, 0.40f, 0.15f, 1.0f);
    Color bronze_dark = color4(0.32f, 0.22f, 0.08f, 1.0f);
    Color bronze_lux  = color4(0.80f, 0.65f, 0.30f, 1.0f);

    float raggio = fmaxf(ctx.rx, ctx.ry) + 4.0f;
    tabula_pinge_sdf(t, ctx.c, raggio, sdf_galea_dome, &ctx, bronze);

    /* Umbra sub margine */
    tabula_pinge_lineam(
        t,
        v2(ctx.c.x - ctx.rx, ctx.y_cut),
        v2(ctx.c.x + ctx.rx, ctx.y_cut),
        2.0f, bronze_dark
    );

    /* Reflectio lucis super */
    tabula_pinge_lineam(
        t,
        v2(ctx.c.x - ctx.rx * 0.35f, ctx.c.y - ctx.ry * 0.60f),
        v2(ctx.c.x + ctx.rx * 0.15f, ctx.c.y - ctx.ry * 0.75f),
        1.4f, bronze_lux
    );

    /* Crista equina — plumae rubrae super */
    Color crista      = color4(0.70f, 0.15f, 0.15f, 1.0f);
    Color crista_dark = color4(0.45f, 0.08f, 0.08f, 1.0f);
    /* Crista ut seriem semicirculorum */
    float cr_y = ctx.c.y - ctx.ry - 2.0f;
    for (int i = 0; i < 9; i++) {
        float u = (float)i / 8.0f;
        float cx = ctx.c.x - ctx.rx * 0.30f + u * ctx.rx * 0.60f;
        float cy = cr_y - sinf(u * PORTRAIT_PI) * 4.0f;
        Color c = (i & 1) ? crista : crista_dark;
        tabula_pinge_discum(t, v2(cx, cy), 2.2f, c);
    }
    /* Linea basis cristae */
    tabula_pinge_lineam(
        t,
        v2(ctx.c.x - ctx.rx * 0.30f, cr_y + 1.0f),
        v2(ctx.c.x + ctx.rx * 0.30f, cr_y + 1.0f),
        1.5f, bronze_dark
    );

    /* Nasale — thin vertical ridge down center */
    tabula_pinge_lineam(
        t,
        v2(ctx.c.x, ctx.y_cut - 1.0f),
        v2(ctx.c.x, ctx.y_cut + z->alt_faciei * 0.10f),
        1.2f, bronze_dark
    );
}

/* --- Vitta: fascia horizontalis circum caput ad frontem --- */

static void vitta(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z) {
    Color cloth;
    Color trim;
    /* Cloth color: white/off-white with variation */
    HSL c_hsl = { 0.10f, 0.10f, 0.85f };
    cloth = hsl_ad_color(c_hsl, 1.0f);
    /* Trim: purpura aut rubrum */
    HSL t_hsl = { saturatef(p->color_capitis_h * 0.2f + 0.90f), 0.70f, 0.32f };
    trim = hsl_ad_color(t_hsl, 1.0f);

    float y_band = z->frons.y_frons + 2.0f;
    float half_w = z->lat_faciei * 1.02f;
    float band_thick = 4.5f;

    /* Main band */
    tabula_pinge_lineam(
        t,
        v2(z->centrum_faciei.x - half_w, y_band),
        v2(z->centrum_faciei.x + half_w, y_band),
        band_thick, cloth
    );

    /* Borders — trim superius et inferius */
    tabula_pinge_lineam(
        t,
        v2(z->centrum_faciei.x - half_w, y_band - band_thick * 0.5f + 0.5f),
        v2(z->centrum_faciei.x + half_w, y_band - band_thick * 0.5f + 0.5f),
        0.8f, trim
    );
    tabula_pinge_lineam(
        t,
        v2(z->centrum_faciei.x - half_w, y_band + band_thick * 0.5f - 0.5f),
        v2(z->centrum_faciei.x + half_w, y_band + band_thick * 0.5f - 0.5f),
        0.8f, trim
    );

    /* Catenae pendulae ad latera */
    for (int s = 0; s < 2; s++) {
        float dir = (s == 0) ? -1.0f : 1.0f;
        vec2 top = v2(z->centrum_faciei.x + dir * half_w * 0.92f, y_band + band_thick * 0.3f);
        vec2 bot = v2(top.x + dir * 1.5f, top.y + z->alt_faciei * 0.25f);
        tabula_pinge_lineam(t, top, bot, 2.0f, cloth);
    }
}

/* --- Pileus: pileus libertatis, conicus apice flexo ante --- */

typedef struct {
    vec2 base_c;
    float base_rx, base_ry;
    vec2 apex;
    float thick;
}PileusCtx;

static void pileus(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z) {
    (void)p;
    Color red      = color4(0.68f, 0.14f, 0.14f, 1.0f);
    Color red_dark = color4(0.45f, 0.08f, 0.08f, 1.0f);
    Color red_lux  = color4(0.85f, 0.28f, 0.22f, 1.0f);

    vec2 base_c = v2(z->centrum_faciei.x, z->frons.y_frons - 1.0f);
    float base_rx = z->lat_faciei * 0.95f;
    float base_ry = z->alt_faciei * 0.14f;

    /* Pingit basim ut ellipsin solidam */
    int N_body = 24;
    vec2 apex = v2(
        z->centrum_faciei.x - z->lat_faciei * 0.35f,
        z->frons.y_culmen - z->alt_faciei * 0.20f
    );

    /* Stratum conicum: seriem capsulae ab basi ad apicem */
    for (int i = 0; i < N_body; i++) {
        float u = (float)i / (float)(N_body - 1);
        float cx = mixf(z->centrum_faciei.x, apex.x, u);
        float cy = mixf(base_c.y, apex.y, u);
        float rw = mixf(base_rx, 2.0f, u);
        float rh = mixf(base_ry, 1.5f, u);
        tabula_pinge_lineam(t, v2(cx - rw, cy), v2(cx + rw, cy), rh * 2.0f, red);
    }

    /* Umbra sub pileo */
    tabula_pinge_lineam(
        t, v2(base_c.x - base_rx, base_c.y + base_ry),
        v2(base_c.x + base_rx, base_c.y + base_ry),
        1.4f, red_dark
    );

    /* Reflectio lucis super caput */
    tabula_pinge_lineam(
        t,
        v2(base_c.x - base_rx * 0.4f, z->frons.y_culmen - z->alt_faciei * 0.05f),
        v2(base_c.x - base_rx * 0.1f, z->frons.y_culmen - z->alt_faciei * 0.15f),
        1.5f, red_lux
    );

    /* Tassel in apice — parvum sphaerulum */
    tabula_pinge_discum(t, apex, 2.0f, red_dark);
}

/* --- Diadema: circulus aureus cum gemma centrale --- */

static void diadema(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z) {
    (void)p;
    Color gold      = color4(0.92f, 0.78f, 0.18f, 1.0f);
    Color gold_dark = color4(0.60f, 0.45f, 0.10f, 1.0f);
    Color gem       = color4(0.78f, 0.12f, 0.22f, 1.0f);

    /* Arcus circulum ad hairline */
    float y = z->frons.y_frons + 1.0f;
    float rx = z->lat_faciei * 1.00f;
    float ry = z->alt_faciei * 0.18f;

    /* Arcus per seriem discorum */
    int N = 24;
    for (int i = 0; i < N; i++) {
        float u = (float)i / (float)(N - 1);
        float ang = mixf(PORTRAIT_PI * 1.02f, -PORTRAIT_PI * 0.02f, u);
        float x = z->centrum_faciei.x + cosf(ang) * rx;
        float dy = sinf(ang) * ry;
        float cy = y + dy * 0.5f;
        tabula_pinge_discum(t, v2(x, cy), 1.2f, gold);
    }

    /* Ornamenta intermedia (puncta auri obscura) */
    for (int i = 0; i < 5; i++) {
        float u = (float)i / 4.0f;
        float ang = mixf(PORTRAIT_PI * 1.02f, -PORTRAIT_PI * 0.02f, u);
        float x = z->centrum_faciei.x + cosf(ang) * rx;
        float cy = y + sinf(ang) * ry * 0.5f;
        tabula_pinge_discum(t, v2(x, cy), 1.8f, gold_dark);
        tabula_pinge_discum(t, v2(x, cy), 1.0f, gold);
    }

    /* Gemma centralis */
    vec2 gem_c = v2(z->centrum_faciei.x, y - 1.5f);
    tabula_pinge_discum(t, gem_c, 3.0f, gold_dark);
    tabula_pinge_discum(t, gem_c, 2.2f, gem);
    /* Glint */
    tabula_pinge_discum(
        t, v2(gem_c.x - 0.7f, gem_c.y - 0.7f), 0.7f,
        color4(1.0f, 0.7f, 0.7f, 0.9f)
    );
}

/* --- Dispatcher --- */

void redde_ornamenta_capitis(
    Tabula* t, const FaciesParametra* p,
    const ZonaeFaciei* z, const PalettaFaciei* col
) {
    (void)col;
    switch (p->modus_ornamenti) {
    case ORNAMENTUM_CORONA_LAUREA: corona_laurea(t, p, z); break;
    case ORNAMENTUM_GALEA:         galea(t, p, z);         break;
    case ORNAMENTUM_VITTA:         vitta(t, p, z);         break;
    case ORNAMENTUM_PILEUS:        pileus(t, p, z);        break;
    case ORNAMENTUM_DIADEMA:       diadema(t, p, z);       break;
    default: break;
    }
}

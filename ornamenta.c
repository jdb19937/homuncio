#include "ornamenta.h"
#include "color.h"
#include "sdf.h"
#include "sors.h"

/* --- Corona: pluribus variantibus Romanis (laurea, civica, myrtea, floralis, spicea) --- */

typedef enum {
    CR_LAUREA = 0,   /* laurus — viridis olivaceus, baccae rubrae — poetae, imperatores */
    CR_CIVICA,       /* quercus — glandes brunneae — pro cive servato */
    CR_MYRTEA,       /* myrtus — viridis obscurior, baccae caeruleae — Veneris, ovationis */
    CR_FLORALIS,     /* flores rosae vel violae — festa, symposia */
    CR_SPICEA,       /* spicae tritici — aurea stantes — Cereris */
    CR_OLIVAE,       /* olea — argenteo-viridis, baccae nigrae — pax, Minervae */
    CR_NUMERUS
} CoronaTypus;

static void corona_laurea(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z) {
    /* Typus ponderatus: variantes non-virides saepius eliguntur ut
     * dominantia viridis (4 ex 6 variantium natūrāliter virentēs) minuātur. */
    Sors s_form = sors_deriva(p->semen, 0xC0F0Au);
    float pond_cr[CR_NUMERUS];
    pond_cr[CR_LAUREA]   = 0.15f;   /* virens */
    pond_cr[CR_CIVICA]   = 0.10f;   /* virens */
    pond_cr[CR_MYRTEA]   = 0.10f;   /* virens obscurus */
    pond_cr[CR_OLIVAE]   = 0.10f;   /* virens argenteus */
    pond_cr[CR_FLORALIS] = 0.30f;   /* roseus/violaceus — non virens */
    pond_cr[CR_SPICEA]   = 0.25f;   /* aureus — non virens */
    CoronaTypus typus = (CoronaTypus) sors_ponderatus(&s_form, pond_cr, CR_NUMERUS);

    Color leaf_base, leaf_dark, leaf_light, berry_c;
    int has_berry = 0;
    switch (typus) {
    case CR_LAUREA:
        leaf_base  = color4(0.20f, 0.45f, 0.18f, 1.0f);
        leaf_dark  = color4(0.10f, 0.25f, 0.10f, 1.0f);
        leaf_light = color4(0.35f, 0.60f, 0.25f, 1.0f);
        berry_c    = color4(0.65f, 0.15f, 0.15f, 1.0f);
        has_berry  = 1;
        break;
    case CR_CIVICA:
        leaf_base  = color4(0.28f, 0.45f, 0.15f, 1.0f);
        leaf_dark  = color4(0.18f, 0.28f, 0.08f, 1.0f);
        leaf_light = color4(0.50f, 0.62f, 0.22f, 1.0f);
        berry_c    = color4(0.45f, 0.30f, 0.12f, 1.0f);  /* glans */
        has_berry  = 1;
        break;
    case CR_MYRTEA:
        leaf_base  = color4(0.10f, 0.32f, 0.18f, 1.0f);
        leaf_dark  = color4(0.05f, 0.18f, 0.10f, 1.0f);
        leaf_light = color4(0.22f, 0.48f, 0.28f, 1.0f);
        berry_c    = color4(0.15f, 0.18f, 0.45f, 1.0f);
        has_berry  = 1;
        break;
    case CR_FLORALIS: {
            Sors sh = sors_deriva(p->semen, 0xF10AAu);
            float r = sors_f32(&sh);
            if (r < 0.5f) {  /* rosae */
                leaf_base  = color4(0.85f, 0.35f, 0.55f, 1.0f);
                leaf_dark  = color4(0.55f, 0.15f, 0.35f, 1.0f);
                leaf_light = color4(0.98f, 0.65f, 0.75f, 1.0f);
            } else {  /* violae */
                leaf_base  = color4(0.55f, 0.35f, 0.70f, 1.0f);
                leaf_dark  = color4(0.32f, 0.18f, 0.45f, 1.0f);
                leaf_light = color4(0.75f, 0.55f, 0.88f, 1.0f);
            }
            berry_c = color4(0.95f, 0.88f, 0.25f, 1.0f);
            has_berry = 1;
            break;
        }
    case CR_SPICEA:
        leaf_base  = color4(0.80f, 0.65f, 0.18f, 1.0f);  /* aureus tritici */
        leaf_dark  = color4(0.55f, 0.42f, 0.10f, 1.0f);
        leaf_light = color4(0.95f, 0.85f, 0.35f, 1.0f);
        berry_c    = color4(0.65f, 0.50f, 0.12f, 1.0f);
        has_berry  = 0;
        break;
    case CR_OLIVAE:
    default:
        leaf_base  = color4(0.42f, 0.50f, 0.28f, 1.0f);
        leaf_dark  = color4(0.22f, 0.28f, 0.14f, 1.0f);
        leaf_light = color4(0.68f, 0.72f, 0.45f, 1.0f);
        berry_c    = color4(0.08f, 0.08f, 0.10f, 1.0f);  /* olivae nigrae */
        has_berry  = 1;
        break;
    }

    Sors s = sors_deriva(p->semen, 0xC0A0A0u);

    float y_center = z->frons.y_culmen - z->alt_faciei * 0.02f;
    float rx = z->lat_faciei * 1.00f;
    float ry = z->alt_faciei * 0.55f;

    if (typus == CR_FLORALIS) {
        /* Flores: discī circulārēs cum centrō lūciō, nōn folia */
        int n = 11;
        for (int i = 0; i < n; i++) {
            float u = (float)i / (float)(n - 1);
            float ang = mixf(PORTRAIT_PI * 1.05f, -PORTRAIT_PI * 0.05f, u);
            float cx = z->centrum_faciei.x + cosf(ang) * rx;
            float cy = y_center + sinf(ang) * ry * 0.80f;
            float r_pet = 2.8f + sors_f32(&s) * 1.3f;
            /* 5 petala discī intorti */
            Color c = (i & 1) ? leaf_base : leaf_light;
            tabula_pinge_discum(t, v2(cx, cy), r_pet, c);
            tabula_pinge_discum(t, v2(cx, cy), r_pet * 0.4f, berry_c);
            /* folia parva viridia inter flores */
            if (i > 0 && (i & 1)) {
                Color leaf_g = color4(0.20f, 0.40f, 0.18f, 1.0f);
                float tx = -sinf(ang), ty = cosf(ang);
                vec2 la = v2(cx - tx * 3.0f - cosf(ang) * 2.0f, cy - ty * 3.0f - sinf(ang) * 2.0f);
                vec2 lb = v2(cx + tx * 2.0f - cosf(ang) * 2.0f, cy + ty * 2.0f - sinf(ang) * 2.0f);
                tabula_pinge_lineam(t, la, lb, 1.6f, leaf_g);
            }
        }
        return;
    }

    if (typus == CR_SPICEA) {
        /* Spīcae tritici: stantēs verticālēs cum grānīs lateribus */
        int n = 9;
        for (int i = 0; i < n; i++) {
            float u = (float)i / (float)(n - 1);
            float ang = mixf(PORTRAIT_PI * 1.02f, -PORTRAIT_PI * 0.02f, u);
            float cx = z->centrum_faciei.x + cosf(ang) * rx;
            float cy_base = y_center + sinf(ang) * ry * 0.85f;
            float stalk_len = 5.0f + sors_f32(&s) * 2.0f;
            /* caulis */
            vec2 a = v2(cx, cy_base);
            vec2 top = v2(cx + sinf(ang) * 0.8f, cy_base - stalk_len);
            tabula_pinge_lineam(t, a, top, 0.9f, leaf_dark);
            /* grana ut paria discōrum alternantia in caule */
            for (int k = 0; k < 4; k++) {
                float kt = (float)k / 3.0f;
                float gx = mixf(a.x, top.x, kt);
                float gy = mixf(a.y, top.y, kt);
                Color g = (k & 1) ? leaf_base : leaf_light;
                tabula_pinge_discum(t, v2(gx - 1.2f, gy), 1.1f, g);
                tabula_pinge_discum(t, v2(gx + 1.2f, gy), 1.1f, g);
            }
        }
        return;
    }

    /* Foliātae: LAUREA, CIVICA, MYRTEA, OLIVAE */
    int n = 14;
    for (int i = 0; i < n; i++) {
        float u = (float)i / (float)(n - 1);
        float ang = mixf(PORTRAIT_PI * 1.05f, -PORTRAIT_PI * 0.05f, u);
        float cx = z->centrum_faciei.x + cosf(ang) * rx;
        float cy = y_center + sinf(ang) * ry * 0.80f;

        float tx = -sinf(ang);
        float ty = cosf(ang);
        float leaf_len = 4.5f + sors_f32(&s) * 2.0f;
        /* CIVICA: folia latiora; MYRTEA: angustiora */
        float leaf_w = (typus == CR_CIVICA) ? 3.2f : (typus == CR_MYRTEA ? 1.8f : 2.4f);
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
        tabula_pinge_lineam(t, a, b, leaf_w, c);
    }

    if (has_berry && sors_f32(&s) < 0.5f) {
        int nb = (typus == CR_OLIVAE || typus == CR_MYRTEA) ? 4 : 3;
        float br = (typus == CR_CIVICA) ? 1.6f : 1.2f;
        for (int i = 0; i < nb; i++) {
            float ang = sors_spatium(&s, PORTRAIT_PI * 0.8f, PORTRAIT_PI * 0.2f);
            float bx = z->centrum_faciei.x + cosf(ang) * rx * 0.95f;
            float by = y_center + sinf(ang) * ry * 0.75f;
            tabula_pinge_discum(t, v2(bx, by), br, berry_c);
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
    /* Dome */
    GaleaCtx ctx;
    ctx.c.x = z->centrum_faciei.x;
    ctx.c.y = z->frons.y_culmen + z->alt_faciei * 0.08f;
    ctx.rx = z->lat_faciei * 1.18f;
    ctx.ry = z->alt_faciei * 0.72f;
    ctx.y_cut = z->frons.y_frons + z->alt_faciei * 0.04f;  /* just above eyebrows */

    /* Metallum per color_capitis_h: aes, ferrum, argentum, cuprum, nigrum */
    Color bronze, bronze_dark, bronze_lux;
    int metallum = (int)(p->color_capitis_h * 5.0f);
    if (metallum > 4)
        metallum = 4;
    if (metallum < 0)
        metallum = 0;
    switch (metallum) {
    case 0: /* aes (yellow-gold bronze) */
        bronze      = color4(0.55f, 0.40f, 0.15f, 1.0f);
        bronze_dark = color4(0.32f, 0.22f, 0.08f, 1.0f);
        bronze_lux  = color4(0.80f, 0.65f, 0.30f, 1.0f);
        break;
    case 1: /* ferrum (steel gray) */
        bronze      = color4(0.42f, 0.42f, 0.46f, 1.0f);
        bronze_dark = color4(0.22f, 0.22f, 0.26f, 1.0f);
        bronze_lux  = color4(0.72f, 0.72f, 0.78f, 1.0f);
        break;
    case 2: /* argentum (silver) */
        bronze      = color4(0.72f, 0.74f, 0.78f, 1.0f);
        bronze_dark = color4(0.48f, 0.50f, 0.55f, 1.0f);
        bronze_lux  = color4(0.92f, 0.93f, 0.95f, 1.0f);
        break;
    case 3: /* cuprum (copper red-orange) */
        bronze      = color4(0.62f, 0.32f, 0.20f, 1.0f);
        bronze_dark = color4(0.38f, 0.18f, 0.10f, 1.0f);
        bronze_lux  = color4(0.85f, 0.52f, 0.36f, 1.0f);
        break;
    default: /* nigrum (black iron) */
        bronze      = color4(0.22f, 0.22f, 0.25f, 1.0f);
        bronze_dark = color4(0.08f, 0.08f, 0.10f, 1.0f);
        bronze_lux  = color4(0.45f, 0.45f, 0.50f, 1.0f);
        break;
    }

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

/* --- Fex: pileum cylindricum truncatum cum fimbria --- */

static void fex(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z) {
    /* Color ex color_capitis_h: ruber (classicus) 50%, alii 50% */
    Sors s = sors_deriva(p->semen, 0xFE6EEu);
    float roll = sors_f32(&s);
    Color body, body_dark, body_lux;
    if (roll < 0.50f) {
        /* ruber classicus */
        body      = color4(0.58f, 0.10f, 0.10f, 1.0f);
        body_dark = color4(0.35f, 0.05f, 0.05f, 1.0f);
        body_lux  = color4(0.78f, 0.20f, 0.15f, 1.0f);
    } else if (roll < 0.70f) {
        body      = color4(0.15f, 0.15f, 0.20f, 1.0f);  /* niger */
        body_dark = color4(0.05f, 0.05f, 0.08f, 1.0f);
        body_lux  = color4(0.35f, 0.35f, 0.40f, 1.0f);
    } else if (roll < 0.85f) {
        body      = color4(0.12f, 0.20f, 0.45f, 1.0f);  /* caeruleus */
        body_dark = color4(0.05f, 0.10f, 0.28f, 1.0f);
        body_lux  = color4(0.30f, 0.42f, 0.65f, 1.0f);
    } else {
        body      = color4(0.40f, 0.30f, 0.10f, 1.0f);  /* fulvus/brunneus */
        body_dark = color4(0.22f, 0.16f, 0.05f, 1.0f);
        body_lux  = color4(0.62f, 0.50f, 0.22f, 1.0f);
    }

    /* Geōmetria: cylindricum truncatum — lātius et altius quam cranium, ut anguli comae non excedant */
    float base_y   = z->frons.y_frons + 1.0f;
    float top_y    = z->frons.y_culmen - z->alt_faciei * 0.28f;
    float base_hw  = z->lat_faciei * 1.18f;
    float top_hw   = z->lat_faciei * 1.12f;

    /* Corpus: plenum rectangulum cum sphaeris rotundatīs */
    int N = 18;
    for (int i = 0; i < N; i++) {
        float u = (float)i / (float)(N - 1);
        float cy = mixf(base_y, top_y, u);
        float hw = mixf(base_hw, top_hw, u);
        /* Paulō sinus in medio — shade variatus */
        Color c = body;
        if (u < 0.15f) c = body_dark;      /* umbra basi */
        else if (u > 0.85f) c = body_lux;  /* lux top */
        tabula_pinge_lineam(t, v2(z->centrum_faciei.x - hw, cy),
                            v2(z->centrum_faciei.x + hw, cy), 2.2f, c);
    }

    /* Margō rotundus in summo — ellipsis parva */
    tabula_pinge_lineam(
        t, v2(z->centrum_faciei.x - top_hw, top_y),
        v2(z->centrum_faciei.x + top_hw, top_y), 2.5f, body_dark
    );

    /* Reflectio lucis super latus sinistrum */
    tabula_pinge_lineam(
        t, v2(z->centrum_faciei.x - base_hw * 0.7f, base_y + 4.0f),
        v2(z->centrum_faciei.x - base_hw * 0.55f, top_y + 4.0f),
        1.6f, body_lux
    );

    /* Fimbria pendens ex summo — chorda + nodus + fila */
    Color tassel_base = body_dark;
    Color tassel_cord = body;
    float tx = z->centrum_faciei.x + top_hw * 0.20f;  /* offset dextram */
    float ty = top_y;
    /* Chorda ex top ad latus */
    vec2 cord_a = v2(tx, ty);
    vec2 cord_b = v2(tx + base_hw * 0.35f, ty + z->alt_faciei * 0.12f);
    tabula_pinge_lineam(t, cord_a, cord_b, 1.2f, tassel_cord);
    /* Nodus medius */
    tabula_pinge_discum(t, cord_b, 1.8f, tassel_base);
    /* Fila pendentia — 5 līneae breves */
    for (int i = 0; i < 5; i++) {
        float fx_off = (i - 2) * 1.0f;
        vec2 fa = v2(cord_b.x + fx_off, cord_b.y + 1.0f);
        vec2 fb = v2(cord_b.x + fx_off * 1.5f,
                     cord_b.y + z->alt_faciei * 0.10f + sors_spatium(&s, -1.0f, 2.0f));
        tabula_pinge_lineam(t, fa, fb, 0.9f, tassel_cord);
    }
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
    case ORNAMENTUM_FEX:           fex(t, p, z);           break;
    default: break;
    }
}

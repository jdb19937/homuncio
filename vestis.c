#include "vestis.h"
#include "color.h"
#include "sdf.h"
#include <stdlib.h>

/* SDF vestis: trapezium angulis rotundis, cum collo rotundō vel V-formī cavō. */
typedef struct {
    float cx;
    float half_top, half_bot;    /* latitudinum dimidiae ad neckline et imum */
    float y_top, y_bot;
    float neck_w, neck_h;        /* magnitudo apertionis colli (semi-axes) */
    int   neck_style;            /* 0=rotundum, 1=V, 2=quadratum */
} VestisCtx;

static float sdf_vestis(vec2 p, void* ctx_) {
    VestisCtx* ctx = (VestisCtx*) ctx_;
    float dx = p.x - ctx->cx;
    float u = clampf((p.y - ctx->y_top) / (ctx->y_bot - ctx->y_top + 1e-4f), 0.0f, 1.0f);
    float half_w = mixf(ctx->half_top, ctx->half_bot, u);

    /* Trapezoid SDF */
    float d_x = fabsf(dx) - half_w;
    float d_y_top = ctx->y_top - p.y;
    float d_y_bot = p.y - ctx->y_bot;
    float dy = fmaxf(d_y_top, d_y_bot);
    float d_trap;
    if (d_x < 0.0f && dy < 0.0f) {
        d_trap = fmaxf(d_x, dy);
    } else {
        float ex = fmaxf(d_x, 0.0f);
        float ey = fmaxf(dy, 0.0f);
        d_trap = sqrtf(ex*ex + ey*ey);
    }

    /* Apertio colli — subtrahit ex summā */
    float neck_dy = p.y - ctx->y_top + 1.0f;   /* positivum sub neckline */
    float d_neck;
    if (ctx->neck_style == 1) {
        /* V: triangulum inversum */
        float vslope = 0.60f;
        float val = fabsf(dx) - (ctx->neck_w - neck_dy * vslope);
        /* intus si val<0 && neck_dy<ctx->neck_h */
        d_neck = fmaxf(val, neck_dy - ctx->neck_h);
    } else if (ctx->neck_style == 2) {
        /* Quadrātum: capsa simplex */
        float cx_d = fabsf(dx) - ctx->neck_w;
        float cy_d = fabsf(neck_dy - ctx->neck_h * 0.5f) - ctx->neck_h * 0.5f;
        d_neck = fmaxf(cx_d, cy_d);
    } else {
        /* rotundum: ellipsis */
        vec2 nq = v2(dx, neck_dy - ctx->neck_h * 0.35f);
        d_neck = sdf_ellipsis(nq, ctx->neck_w, ctx->neck_h * 0.75f);
    }

    return sdf_subtractio(d_trap, d_neck);
}

/* Color vestis ex parametris */
static Color vestis_color_base(const FaciesParametra* p) {
    HSL h = { saturatef(p->color_vestis_h),
        saturatef(p->color_vestis_s),
        saturatef(p->color_vestis_v) };
    return hsl_ad_color(h, 1.0f);
}

static Color vestis_color_ornamenti(const FaciesParametra* p) {
    HSL h = { saturatef(p->color_ornamenti_h), 0.75f, 0.30f };
    return hsl_ad_color(h, 1.0f);
}

void redde_vestis(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    if (p->modus_vestis == VESTIS_NULLA)
        return;

    float cx = z->centrum_faciei.x;
    float y_top = z->cervix.y_sup + z->alt_faciei * 0.18f;

    VestisCtx ctx;
    ctx.cx        = cx;
    ctx.half_top  = z->lat_faciei * 0.48f;
    ctx.half_bot  = z->lat_faciei * 1.45f;
    ctx.y_top     = y_top;
    ctx.y_bot     = (float) t->h + 4.0f;
    ctx.neck_w    = z->lat_faciei * 0.22f;
    ctx.neck_h    = z->alt_faciei * 0.10f;
    ctx.neck_style = 0;    /* rotundus defaulta */

    Color base   = vestis_color_base(p);
    Color dark   = color_obscurior(base, 0.30f);
    Color light  = color_clariorem(base, 0.15f);
    Color orn    = vestis_color_ornamenti(p);

    /* Formam specificam — mutat contextum et palettam */
    switch (p->modus_vestis) {
    case VESTIS_TUNICA:
        ctx.neck_style = 0;
        ctx.neck_w = z->lat_faciei * 0.22f;
        ctx.neck_h = z->alt_faciei * 0.08f;
        break;
    case VESTIS_TOGA:
        ctx.half_top = z->lat_faciei * 0.65f;
        ctx.half_bot = z->lat_faciei * 1.50f;
        ctx.neck_style = 0;
        ctx.neck_w = z->lat_faciei * 0.28f;
        ctx.neck_h = z->alt_faciei * 0.06f;
        break;
    case VESTIS_STOLA:
        ctx.neck_style = 1;   /* V-collum */
        ctx.neck_w = z->lat_faciei * 0.30f;
        ctx.neck_h = z->alt_faciei * 0.20f;
        break;
    case VESTIS_LORICA:
        ctx.neck_style = 2;   /* quadratum */
        ctx.neck_w = z->lat_faciei * 0.30f;
        ctx.neck_h = z->alt_faciei * 0.08f;
        ctx.half_top = z->lat_faciei * 0.60f;
        break;
    case VESTIS_PALLIUM:
        ctx.neck_style = 0;
        ctx.neck_w = z->lat_faciei * 0.20f;
        ctx.neck_h = z->alt_faciei * 0.08f;
        break;
    default: return;
    }

    /* Pingit corpus fundamentalem vestimenti */
    vec2 paint_c = v2(cx, (y_top + ctx.y_bot) * 0.5f);
    float paint_r = fmaxf(ctx.half_bot, (ctx.y_bot - y_top) * 0.5f) + 4.0f;
    tabula_pinge_sdf(t, paint_c, paint_r, sdf_vestis, &ctx, base);

    /* Ornamenta et details per vestis specificum */
    switch (p->modus_vestis) {
    case VESTIS_TUNICA: {
            /* Limbus obscurior ad collum + fascia cīnctī ad medium */
            Color trim = dark;
            trim.a = 0.7f;
            /* Cinctus ad umbilicum */
            float belt_y = y_top + z->alt_faciei * 0.55f;
            if (belt_y < (float)t->h) {
                float bw = mixf(
                    ctx.half_top, ctx.half_bot,
                    (belt_y - y_top) / (ctx.y_bot - y_top)
                );
                tabula_pinge_lineam(
                    t, v2(cx - bw, belt_y), v2(cx + bw, belt_y),
                    2.5f, trim
                );
            }
            /* Linea umeri */
            tabula_pinge_discum(
                t, v2(cx - ctx.half_top * 0.6f, y_top + 1.0f), 2.5f,
                color_obscurior(base, 0.15f)
            );
            tabula_pinge_discum(
                t, v2(cx + ctx.half_top * 0.6f, y_top + 1.0f), 2.5f,
                color_obscurior(base, 0.15f)
            );
            break;
        }
    case VESTIS_TOGA: {
            /* Plicae diagōnāles super pectus */
            Color fold = color_obscurior(base, 0.25f);
            fold.a = 0.85f;
            for (int i = 0; i < 5; i++) {
                float y_a = y_top + z->alt_faciei * (0.05f + (float)i * 0.15f);
                float y_b = y_a + z->alt_faciei * 0.35f;
                if (y_a > (float)t->h)
                    break;
                vec2 a = v2(cx - z->lat_faciei * 1.20f, y_a);
                vec2 b = v2(cx + z->lat_faciei * 0.90f, y_b);
                tabula_pinge_lineam(t, a, b, 1.0f, fold);
            }
            /* Clavus: stria purpurea verticālis */
            float stripe_x = cx + z->lat_faciei * 0.30f;
            tabula_pinge_lineam(
                t, v2(stripe_x, y_top + 2.0f),
                v2(stripe_x + 2.0f, (float)t->h), 4.0f, orn
            );
            break;
        }
    case VESTIS_STOLA: {
            /* Fibula aurea in collo */
            Color gold = color4(0.95f, 0.78f, 0.22f, 1.0f);
            tabula_pinge_discum(t, v2(cx, y_top + ctx.neck_h + 1.0f), 2.8f, gold);
            tabula_pinge_discum(
                t, v2(cx, y_top + ctx.neck_h + 1.0f), 1.5f,
                color_obscurior(gold, 0.5f)
            );
            /* Plicae verticāles leniter */
            Color fold = color_obscurior(base, 0.15f);
            fold.a = 0.7f;
            for (int i = 0; i < 4; i++) {
                float fx = cx - z->lat_faciei * 0.8f
                    + (float)i * z->lat_faciei * 0.55f;
                float fy_top = y_top + z->alt_faciei * 0.30f;
                tabula_pinge_lineam(
                    t, v2(fx + 1.0f, fy_top),
                    v2(fx, (float)t->h), 1.0f, fold
                );
            }
            break;
        }
    case VESTIS_LORICA: {
            /* Bandae horizontāles segmentātae */
            Color edge  = color_obscurior(base, 0.55f);
            Color gleam = color_clariorem(base, 0.35f);
            int n_bands = 4;
            for (int i = 0; i < n_bands; i++) {
                float by = y_top + z->alt_faciei * (0.10f + (float)i * 0.14f);
                if (by > (float)t->h)
                    break;
                float bw = mixf(
                    ctx.half_top, ctx.half_bot,
                    (by - y_top) / (ctx.y_bot - y_top)
                );
                tabula_pinge_lineam(
                    t, v2(cx - bw, by), v2(cx + bw, by),
                    1.2f, edge
                );
                /* Reflectiō lūminis */
                tabula_pinge_lineam(
                    t, v2(cx - bw * 0.75f, by + 1.5f),
                    v2(cx - bw * 0.10f, by + 1.5f),
                    0.8f, gleam
                );
            }
            /* Pauldrones: discī super umerōs */
            int n_p = 4;
            for (int i = 0; i < n_p; i++) {
                float px_l = cx - z->lat_faciei * (0.80f - (float)i * 0.22f);
                float px_r = cx + z->lat_faciei * (0.80f - (float)i * 0.22f);
                tabula_pinge_discum(t, v2(px_l, y_top + 1.0f), 3.2f, dark);
                tabula_pinge_discum(t, v2(px_l - 0.8f, y_top), 1.5f, gleam);
                tabula_pinge_discum(t, v2(px_r, y_top + 1.0f), 3.2f, dark);
                tabula_pinge_discum(t, v2(px_r + 0.8f, y_top), 1.5f, gleam);
            }
            break;
        }
    case VESTIS_PALLIUM: {
            /* Drapa asymmetrica: plica diagōnālis dextrā-sursum ad sinistrā-deorsum */
            Color fold = color_obscurior(base, 0.35f);
            fold.a = 0.85f;
            vec2 a1 = v2(
                cx - z->lat_faciei * 1.20f,
                y_top + z->alt_faciei * 0.15f
            );
            vec2 b1 = v2(cx + z->lat_faciei * 0.60f, (float)t->h);
            tabula_pinge_lineam(t, a1, b1, 2.5f, fold);
            vec2 a2 = v2(
                cx - z->lat_faciei * 0.50f,
                y_top + z->alt_faciei * 0.28f
            );
            vec2 b2 = v2(cx + z->lat_faciei * 1.10f, (float)t->h);
            tabula_pinge_lineam(t, a2, b2, 1.8f, fold);
            /* Clasp/fibula super umerum */
            Color clasp = color4(0.92f, 0.78f, 0.18f, 1.0f);
            tabula_pinge_discum(
                t, v2(
                    cx - z->lat_faciei * 0.60f,
                    y_top + z->alt_faciei * 0.05f
                ),
                3.5f, clasp
            );
            tabula_pinge_discum(
                t, v2(
                    cx - z->lat_faciei * 0.60f,
                    y_top + z->alt_faciei * 0.05f
                ),
                1.8f, color_obscurior(clasp, 0.55f)
            );
            break;
        }
    default: break;
    }

    /* Umbra subtilis sub collō ex ipsō vestimentō */
    (void)col;
    (void)light;
}

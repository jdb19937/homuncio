#include "coma.h"
#include "sdf.h"
#include "tumultus.h"
#include "sors.h"
#include "color.h"

/* ------------------------------------------------------------------ */
/* MASSA COMAE                                                         */
/* ------------------------------------------------------------------ */

/* Coma conformis: cappa super cranium, plus (pro comis longis) fasciae descendentes
 * iuxta faciem. SDF est unio lenis lapellorum geometricorum. */
typedef struct {
    vec2  centrum_faciei;    /* eodem sito ut face SDF */
    float face_lat, face_alt;

    float cap_thick;         /* crassitudo stratis super caput */
    float cap_poof;          /* extensio sursum ultra cranium */
    float cap_peak_off;      /* recentium centri superioris prop. volumine */

    float descent_len;       /* longitudo fasciarum lateralium dextra/sinistra */
    float descent_wide;      /* dimidia latitudo fasciarum */

    float hairline_drop;     /* quantum hairline descendat sub fronte (recessus) */

    float vent_amp;
    float freq_dist;
    uint32_t semen_fbm;

    int   parted_left;       /* si coma parsa ad sinistram partem */
    float part_strength;     /* 0 = nulla parsa, 1 = parsa plena */
} ComaCtx;

/* Evalua SDF comae ad positionem absolutam p. Negativum si intus hair, positivum extra. */
static float sdf_coma(vec2 p, void* ctx_) {
    ComaCtx* ctx = (ComaCtx*) ctx_;
    vec2 q = v2_sub(p, ctx->centrum_faciei);

    /* Distortio per FBM — organicum "hair texture" */
    vec2 dd = distortio_regionis(
        v2(q.x * ctx->freq_dist, q.y * ctx->freq_dist),
        2.0f + 3.0f * ctx->vent_amp, ctx->semen_fbm
    );
    q = v2_add(q, dd);

    /* Cranium ellipsis: face ellipse augmentata per cap_thick, cum peak supra */
    vec2 skull_q = v2(q.x, q.y + ctx->cap_peak_off);
    float skull_lat = ctx->face_lat + ctx->cap_thick;
    float skull_alt = ctx->face_alt + ctx->cap_thick + ctx->cap_poof;
    float d_skull = sdf_ellipsis(skull_q, skull_lat, skull_alt);

    /* Linea hair—cut bassa in q.y (absolute). Hairline est sub sumo frontis. */
    /* Hairline_y sub centrum ≈ -(face_alt * 0.45 - hairline_drop) */
    float hairline_q_y = -ctx->face_alt * 0.48f + ctx->hairline_drop;

    /* Parsa: si parted_left, in regione iuxta partem, hairline_drop plus magna */
    if (ctx->part_strength > 0.01f) {
        float part_x = (ctx->parted_left ? 1.0f : -1.0f) * ctx->face_lat * 0.08f;
        float near_part = fmaxf(0.0f, 1.0f - fabsf(q.x - part_x) / (ctx->face_lat * 0.18f));
        hairline_q_y += near_part * ctx->part_strength * 3.0f;
    }

    /* Cut region below hairline: d_cut positive sub hairline, negative supra.
     * Intersect: hair exists solum ubi q.y < hairline_q_y (i.e., supra). */
    float d_cut = q.y - hairline_q_y;
    float d_cap = sdf_intersectio(d_skull, d_cut);

    /* Fasciae descendentes laterales */
    if (ctx->descent_len > 3.0f) {
        float side_x = ctx->face_lat * 0.88f;
        /* Top ad hairline altum, bot ad hairline + descent_len */
        float top_y = hairline_q_y - 1.0f;
        float bot_y = hairline_q_y + ctx->descent_len;
        float wide  = ctx->descent_wide;

        /* Sinistra */
        vec2 a_l = v2(-side_x, top_y);
        vec2 b_l = v2(-side_x + ctx->vent_amp * 1.5f, bot_y);
        float d_left = sdf_capsula(q, a_l, b_l, wide);

        /* Dextra */
        vec2 a_r = v2(side_x, top_y);
        vec2 b_r = v2(side_x - ctx->vent_amp * 1.5f, bot_y);
        float d_right = sdf_capsula(q, a_r, b_r, wide);

        d_cap = sdf_unio_lenis(d_cap, d_left,  3.0f);
        d_cap = sdf_unio_lenis(d_cap, d_right, 3.0f);
    }

    return d_cap;
}

void redde_coma_massa(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    if (p->modus_comae == COMA_CALVA || p->volumen_comae < 0.05f)
        return;

    float volumen   = p->volumen_comae;
    float longitudo = p->longitudo_comae;

    ComaCtx ctx;
    ctx.centrum_faciei = z->centrum_faciei;
    ctx.face_lat = z->lat_faciei;
    ctx.face_alt = z->alt_faciei;

    /* Animatio venti */
    float wind = sinf(PORTRAIT_TAU * p->tempus) * 0.5f + cosf(PORTRAIT_TAU * p->tempus * 2.0f) * 0.25f;
    ctx.vent_amp  = 0.3f + 0.8f * fabsf(wind);
    ctx.freq_dist = 0.08f + 0.04f * volumen;
    ctx.semen_fbm = (uint32_t)(p->semen & 0xFFFFFFFFu);

    /* Sors deterministica pro parsa */
    Sors ss = sors_deriva(p->semen, 0xCABE110u);
    ctx.parted_left  = sors_f32(&ss) < 0.5f;
    ctx.part_strength = sors_f32(&ss) < 0.4f ? sors_spatium(&ss, 0.5f, 1.0f) : 0.0f;

    /* Hairline recessus ex recessus_frontis */
    ctx.hairline_drop = -p->recessus_frontis * z->alt_faciei * 0.25f;

    /* Parametra secundum modum comae — cappa conformis + descent variabilis */
    float base_desc = longitudo * z->alt_faciei * 1.15f;
    switch (p->modus_comae) {
    case COMA_BREVIS_RECTA:
        ctx.cap_thick    = 1.5f + 2.5f * volumen;
        ctx.cap_poof     = 0.5f + 1.0f * volumen;
        ctx.cap_peak_off = 0.0f;
        ctx.descent_len  = 0.0f;
        ctx.descent_wide = 0.0f;
        ctx.part_strength = 0.0f;
        break;
    case COMA_MEDIA_UNDATA:
        ctx.cap_thick    = 2.5f + 3.0f * volumen;
        ctx.cap_poof     = 1.0f + 1.5f * volumen;
        ctx.cap_peak_off = 1.0f * volumen;
        ctx.descent_len  = base_desc * 0.35f;
        ctx.descent_wide = z->lat_faciei * (0.10f + 0.04f * volumen);
        ctx.vent_amp    *= 1.2f;
        break;
    case COMA_LONGA_RECTA:
        ctx.cap_thick    = 2.5f + 2.5f * volumen;
        ctx.cap_poof     = 0.5f + 1.0f * volumen;
        ctx.cap_peak_off = 0.0f;
        ctx.descent_len  = base_desc * 0.95f;
        ctx.descent_wide = z->lat_faciei * (0.12f + 0.03f * volumen);
        break;
    case COMA_CRISPATA:
        ctx.cap_thick    = 3.0f + 5.0f * volumen;
        ctx.cap_poof     = 2.0f + 3.0f * volumen;
        ctx.cap_peak_off = 2.0f * volumen;
        ctx.descent_len  = base_desc * 0.5f;
        ctx.descent_wide = z->lat_faciei * (0.14f + 0.05f * volumen);
        ctx.vent_amp    *= 2.0f;
        ctx.freq_dist    = 0.14f;
        break;
    case COMA_GLOBOSA:
        ctx.cap_thick    = 4.0f + 6.0f * volumen;
        ctx.cap_poof     = 4.0f + 8.0f * volumen;
        ctx.cap_peak_off = 3.0f + 3.0f * volumen;
        ctx.descent_len  = 0.0f;
        ctx.descent_wide = 0.0f;
        ctx.vent_amp    *= 1.5f;
        break;
    case COMA_CRISTA:
        ctx.cap_thick    = 1.5f;
        ctx.cap_poof     = 6.0f + 8.0f * volumen;
        ctx.cap_peak_off = 8.0f + 6.0f * volumen;
        ctx.descent_len  = 0.0f;
        ctx.descent_wide = 0.0f;
        break;
    case COMA_NODUS:
        ctx.cap_thick    = 2.0f + 2.0f * volumen;
        ctx.cap_poof     = 3.0f + 3.0f * volumen;
        ctx.cap_peak_off = 4.0f + 2.0f * volumen;
        ctx.descent_len  = 0.0f;
        ctx.descent_wide = 0.0f;
        break;
    case COMA_EFFUSA:
        ctx.cap_thick    = 3.0f + 4.0f * volumen;
        ctx.cap_poof     = 2.0f + 3.0f * volumen;
        ctx.cap_peak_off = 1.0f * volumen;
        ctx.descent_len  = base_desc * 1.15f;
        ctx.descent_wide = z->lat_faciei * (0.16f + 0.05f * volumen);
        ctx.vent_amp    *= 1.8f;
        break;
    default:
        ctx.cap_thick = 3.0f;
        ctx.cap_poof = 2.0f;
        ctx.cap_peak_off = 1.0f;
        ctx.descent_len = 0.0f;
        ctx.descent_wide = 0.0f;
        break;
    }

    /* Regio pingenda: centrum ad frons.y_culmen, raggio magnus ut totam cappam + fasciam tegat */
    vec2 paint_centrum = v2(
        z->centrum_faciei.x,
        z->centrum_faciei.y - z->alt_faciei * 0.3f
    );
    float paint_r = z->alt_faciei + ctx.cap_poof + ctx.descent_len + 10.0f;

    /* Massa principalis */
    tabula_pinge_sdf(t, paint_centrum, paint_r, sdf_coma, &ctx, col->comae);

    /* Umbra sub cappa — stratum obscurum displicatum leviter */
    ComaCtx umb_ctx = ctx;
    umb_ctx.cap_thick    *= 0.75f;
    umb_ctx.cap_poof     *= 0.5f;
    umb_ctx.cap_peak_off *= 0.5f;
    umb_ctx.descent_len  *= 0.90f;
    umb_ctx.descent_wide *= 0.85f;
    Color umb = col->comae_obscura;
    umb.a = 0.55f;
    /* Displicamus umbra ad sinistram-superiorem prop. luminosa direct. */
    umb_ctx.centrum_faciei.x += 1.5f;
    umb_ctx.centrum_faciei.y += 1.5f;
    tabula_pinge_sdf(t, paint_centrum, paint_r, sdf_coma, &umb_ctx, umb);
}

/* ------------------------------------------------------------------ */
/* FILA COMAE (ictus procedurales super massam)                        */
/* ------------------------------------------------------------------ */

/* Pingit fila capillaria sequentia fluxum FBM per ictus brevēs super massam.
 * Fila tangunt solum regionem capillariam (intra envelop oeum faciei extensum,
 * supra faciem aut iuxta lateraliter pro descendentibus). */
void redde_coma_fila(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    if (p->modus_comae == COMA_CALVA || p->volumen_comae < 0.05f)
        return;

    Sors s = sors_deriva(p->semen, 0xFEEDBEEFu);
    uint32_t semen_fbm = (uint32_t)(p->semen & 0xFFFFFFFFu) ^ 0xF1F1F1F1u;

    int N = 120 + (int)(100.0f * p->volumen_comae + 80.0f * p->longitudo_comae);

    float wind = sinf(PORTRAIT_TAU * p->tempus) * 0.4f;

    /* Regio candidāta: intra circuitum capitis + margo lateralis pro descendentibus */
    float env_lat = z->lat_faciei * 1.18f;
    float env_alt = z->alt_faciei * 1.12f;

    Color c_base   = col->comae;
    Color c_bright = color_clariorem(col->comae, 0.32f);
    Color c_dark   = color_obscurior(col->comae, 0.30f);

    for (int i = 0; i < N; i++) {
        /* Positio fortuita in envelope capillario — ypsilōn biāsat sursum */
        float x = sors_spatium(
            &s,
            z->centrum_faciei.x - env_lat,
            z->centrum_faciei.x + env_lat
        );
        float ty = sors_f32(&s);
        ty = ty * ty;   /* biās supra */
        float y = mixf(
            z->centrum_faciei.y - env_alt,
            z->centrum_faciei.y + z->alt_faciei * p->longitudo_comae * 0.9f,
            ty
        );

        /* Intrā ellipsin envelopī capillāris? */
        vec2 q = v2_sub(v2(x, y), z->centrum_faciei);
        float d_env = sdf_ellipsis(q, env_lat, env_alt);
        if (d_env > 1.5f)
            continue;

        /* Nē super faciem (ubi nulla coma): si in faciē, tolerāmus solum sī extrā
         * centrum faciēī lateraliter (regiōne descendentis) */
        float d_face = sdf_contura_faciei(v2(x, y), p, z);
        if (d_face < -3.0f)
            continue;  /* profundum in faciē */

        /* Sī sub hairlīne (circa y_frons), solum si extrā lātus faciēī */
        if (y > z->frons.y_frons + 2.0f) {
            float dist_from_center = fabsf(q.x);
            if (dist_from_center < z->lat_faciei * 0.75f)
                continue;
        }

        /* Flow direction via FBM gradient */
        vec2 dir = fluxus_directio(x * 0.07f, y * 0.07f, semen_fbm);

        /* Blend with outward-from-crown direction */
        vec2 to_crown = v2(
            z->centrum_faciei.x - x,
            z->centrum_faciei.y - z->alt_faciei * 0.85f - y
        );
        float crown_len = v2_len(to_crown);
        if (crown_len > 0.1f) {
            float ox = -to_crown.x / crown_len;
            float oy = -to_crown.y / crown_len;
            /* Sī iam sub chin-line, biās ad descensum (dy positivum) */
            if (y > z->centrum_faciei.y) {
                oy = 0.90f;
                ox *= 0.3f;
            }
            dir.x = dir.x * 0.55f + ox * 0.45f;
            dir.y = dir.y * 0.55f + oy * 0.45f;
        }

        /* Longitudo filī */
        float len = sors_spatium(&s, 2.5f, 4.5f + 3.5f * p->volumen_comae);

        /* Wind modulat directionem */
        dir.x += wind * 0.25f * sinf(y * 0.12f);

        vec2 end = v2(x + dir.x * len, y + dir.y * len);

        /* Color variatio per FBM */
        float var = fbm_s(x * 0.12f, y * 0.12f, 2, 0.5f, semen_fbm + 31u);
        Color fil_c;
        if (var >  0.20f)
            fil_c = c_bright;
        else if (var < -0.20f)
            fil_c = c_dark;
        else
            fil_c = c_base;
        fil_c.a = 0.55f + 0.20f * sors_f32(&s);

        tabula_pinge_lineam(t, v2(x, y), end, 0.7f, fil_c);
    }
}

/* ------------------------------------------------------------------ */
/* BARBA                                                               */
/* ------------------------------------------------------------------ */

void redde_barba(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    if (p->modus_barbae == BARBA_NULLA)
        return;

    Sors s = sors_deriva(p->semen, 0xBA2BA1234u);
    uint32_t semen_fbm = (uint32_t)(p->semen ^ 0xB3ADu);

    /* Regiones anchor: ab aures (sideburns) ad mentum */
    float ear_y    = z->oculi.y + z->alt_faciei * 0.15f;        /* ubi patillae incipiunt */
    float jaw_y    = z->centrum_faciei.y + z->alt_faciei * 0.55f;
    float chin_y   = z->mentum.centrum.y + z->mentum.r * 0.4f;
    float cheek_y  = z->centrum_faciei.y + z->alt_faciei * 0.25f; /* supra jaw, sub mala */
    float jaw_half = z->lat_faciei * 0.95f;
    float face_cx  = z->centrum_faciei.x;
    float mouth_y  = z->os.centrum.y;

    /* Punctum: densitas altior in regione "pilosa" ex modus_barbae */
    int N;
    float density = p->densitas_barbae;
    float y_top;   /* quota superior samplingī per modum */
    switch (p->modus_barbae) {
    case BARBA_STIRPS:    N = 900;
        density *= 0.45f;
        y_top = cheek_y;
        break;
    case BARBA_MAXILLAE:  N = 1100;
        y_top = ear_y;
        break;
    case BARBA_PLENA:     N = 1600;
        y_top = ear_y;
        break;
    case BARBA_PROLIXA:   N = 2200;
        y_top = ear_y;
        break;
    case BARBA_BIFIDA:    N = 700;
        y_top = jaw_y;
        break;
    default:              N = 500;
        y_top = jaw_y;
        break;
    }
    float y_bot = chin_y + z->alt_faciei * 0.2f
        + ((p->modus_barbae == BARBA_PROLIXA) ? z->alt_faciei * 0.4f : 0.0f);

    for (int i = 0; i < N; i++) {
        float x = sors_spatium(&s, face_cx - jaw_half, face_cx + jaw_half);
        float y = sors_spatium(&s, y_top, y_bot);
        float dx = x - face_cx;
        float dy_chin = y - chin_y;
        /* Distantia ā līneā mandibulae (īdeālī): arcus ab aure per chin */
        float jaw_arc_y = jaw_y + (fabsf(dx) / jaw_half) * (chin_y - jaw_y) * 0.6f;
        float dist_jaw = fabsf(y - jaw_arc_y);

        /* Excludit labia/ōs area (barba nōn crescit in labiīs) */
        if (fabsf(dx) < z->os.lat * 0.9f && fabsf(y - mouth_y) < z->os.alt * 1.6f)
            continue;

        float threshold;
        switch (p->modus_barbae) {
        case BARBA_STIRPS:
            /* Stubble ubique: stabilis ubique */
            threshold = 0.55f;
            break;
        case BARBA_MAXILLAE:
            /* Tantum prope mandibulam + patillae */
            threshold = (dist_jaw < z->alt_faciei * 0.08f) ? 0.30f : 0.92f;
            break;
        case BARBA_PLENA: {
            /* Dense super mandibulam, tapering in genās */
                float cheek_up = fmaxf(0.0f, (jaw_arc_y - y) / (z->alt_faciei * 0.35f));
                threshold = 0.35f + cheek_up * 0.55f;
            /* Paululum densior in mentō */
                if (dy_chin > 0.0f && fabsf(dx) < z->lat_faciei * 0.35f)
                    threshold -= 0.15f;
                break;
            }
        case BARBA_PROLIXA: {
                float cheek_up = fmaxf(0.0f, (jaw_arc_y - y) / (z->alt_faciei * 0.40f));
                threshold = 0.25f + cheek_up * 0.50f;
                if (y > chin_y)
                    threshold = 0.20f;  /* prolixa sub chinem densissima */
                break;
            }
        case BARBA_BIFIDA: {
            /* Duo lobi — vacuum in mediō mentō */
                float gap = z->lat_faciei * 0.10f;
                if (fabsf(dx) < gap)
                    threshold = 0.95f;
                else if (fabsf(dx) < z->lat_faciei * 0.40f && y > jaw_y)
                    threshold = 0.30f;
                else
                    threshold = 0.88f;
                break;
            }
        default: threshold = 0.6f; break;
        }

        float noise_val = fbm_s(x * 0.15f, y * 0.15f, 3, 0.5f, semen_fbm) * 0.5f + 0.5f;
        if (noise_val * density < threshold)
            continue;

        Color b_c = col->comae_obscura;
        if (p->modus_barbae == BARBA_STIRPS) {
            tabula_pinge_discum(t, v2(x, y), 0.35f, b_c);
        } else {
            tabula_pinge_lineam(t, v2(x, y), v2(x, y + 2.0f), 0.6f, b_c);
        }
    }

    /* Barba prolixa — massa sub chin */
    if (p->modus_barbae == BARBA_PROLIXA) {
        vec2 c = v2(z->centrum_faciei.x, chin_y + z->alt_faciei * 0.25f);
        for (int i = 0; i < 30; i++) {
            float u = sors_f32(&s);
            float ang = sors_spatium(&s, -0.8f, 0.8f);
            float l = z->alt_faciei * (0.15f + 0.2f * u);
            vec2 end = v2(c.x + sinf(ang) * l, c.y + cosf(ang) * l);
            tabula_pinge_lineam(t, c, end, 2.0f, col->comae_obscura);
        }
    }
}

/* ------------------------------------------------------------------ */
/* MUSTACIA                                                            */
/* ------------------------------------------------------------------ */

void redde_mustacia(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    if (p->modus_mustaciorum == MUSTACIA_NULLA)
        return;
    vec2 c = v2(z->os.centrum.x, z->os.centrum.y - z->os.alt * 0.9f - 1.0f);
    float lat, alt;
    switch (p->modus_mustaciorum) {
    case MUSTACIA_SIMPLEX:    lat = z->os.lat * 0.9f;
        alt = 1.5f;
        break;
    case MUSTACIA_LATA:       lat = z->os.lat * 1.3f;
        alt = 2.5f;
        break;
    case MUSTACIA_IMPERATORIA:lat = z->os.lat * 1.5f;
        alt = 3.0f;
        break;
    default:                  return;
    }
    /* Corpus central */
    tabula_pinge_lineam(t, v2(c.x - lat, c.y), v2(c.x + lat, c.y), alt, col->comae_obscura);
    /* Ekstensiones laterales curvatae */
    if (p->modus_mustaciorum >= MUSTACIA_LATA) {
        tabula_pinge_lineam(
            t, v2(c.x - lat, c.y),
            v2(c.x - lat - 3.0f, c.y - 2.0f), alt * 0.8f, col->comae_obscura
        );
        tabula_pinge_lineam(
            t, v2(c.x + lat, c.y),
            v2(c.x + lat + 3.0f, c.y - 2.0f), alt * 0.8f, col->comae_obscura
        );
    }
}

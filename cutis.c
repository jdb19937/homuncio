#include "cutis.h"
#include "tumultus.h"
#include "color.h"
#include "sors.h"

/* Helper: sdf adapter */
typedef struct {
    const FaciesParametra* p;
    const ZonaeFaciei*     z;
} FaciesSdfCtx;

static float sdf_facies_adapter(vec2 p, void* ctx_) {
    FaciesSdfCtx* ctx = (FaciesSdfCtx*) ctx_;
    return sdf_contura_faciei(p, ctx->p, ctx->z);
}

/* ------------------------------------------------------------------ */
/* CUTIS BASIS                                                         */
/* ------------------------------------------------------------------ */

void redde_cutis_base(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    FaciesSdfCtx ctx = { p, z };

    /* Cutis principalis */
    tabula_pinge_sdf(
        t, z->centrum_faciei,
        fmaxf(z->lat_faciei, z->alt_faciei) + 4.0f,
        sdf_facies_adapter, &ctx, col->cutis
    );

    /* Umbra lateralis (cheek shadow / jaw shadow) — secundum nasum umbra ad sin */
    /* Obscuratur regio dextra facere illuminationem ex sinistra */
    int w = t->w, h = t->h;
    uint32_t semen_fbm = (uint32_t)(p->semen & 0xFFFFFFFFu) ^ 0x7A7A7A7Au;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            vec2 q = v2((float)x + 0.5f, (float)y + 0.5f);
            float d = sdf_contura_faciei(q, p, z);
            if (d > -1.0f)
                continue;  /* tantum intus facies */
            /* gradient illuminationis ex sinistra ad dextram */
            float rx = (q.x - z->centrum_faciei.x) / z->lat_faciei;
            float ry = (q.y - z->centrum_faciei.y) / z->alt_faciei;

            /* Lumen: dot(normale, lumen_dir) */
            float nx = rx;
            float ny = ry;
            float nl = sqrtf(nx*nx + ny*ny) + 1e-10f;
            nx /= nl;
            ny /= nl;
            /* lumen ex sinistra-superior — attenuatum ne faciem secaret */
            float lumen = -0.40f * nx - 0.20f * ny;
            lumen = saturatef(lumen * 0.5f + 0.5f);

            /* Textura pororum — subtilis */
            float pores = fbm_s(q.x * 0.9f, q.y * 0.9f, 3, 0.5f, semen_fbm) * 0.5f + 0.5f;
            lumen *= mixf(1.0f, 0.93f + 0.12f * pores, p->visibilitas_pororum);

            /* Mixtio inter umbra et lux — opacitas minor ne cutem vincat */
            Color shade = color_misce(col->cutis_umbra, col->cutis_lux, lumen);
            shade.a = 0.45f;
            tabula_misce(t, x, y, shade);

            /* Sub-superficie: rubor in apice nasi, labiis, gena.  Approximatio. */
            float sub = 0.0f;
            /* Apex nasi */
            float nx2 = q.x - z->nasus.centrum.x;
            float ny2 = q.y - z->nasus.centrum.y - z->nasus.alt * 0.35f;
            float dn = sqrtf(nx2*nx2 + ny2*ny2) / (z->nasus.lat * 1.5f + 1e-10f);
            sub = fmaxf(sub, 0.4f * (1.0f - saturatef(dn)));
            /* genae */
            float cx = q.x - z->centrum_faciei.x;
            float cy = q.y - z->oculi.y - z->alt_faciei * 0.25f;
            float cxa = fabsf(cx) - z->lat_faciei * 0.55f;
            if (cxa < 0.0f)
                cxa = 0.0f;
            float cyd = fabsf(cy) - z->alt_faciei * 0.05f;
            if (cyd < 0.0f)
                cyd = 0.0f;
            float cd = sqrtf(cxa*cxa + cyd*cyd) / (z->lat_faciei * 0.25f + 1e-10f);
            sub = fmaxf(sub, 0.3f * (1.0f - saturatef(cd)));

            if (sub > 0.01f) {
                Color rub = color4(col->cutis.r * 1.15f, col->cutis.g * 0.92f, col->cutis.b * 0.90f, 0.6f * sub);
                tabula_misce(t, x, y, rub);
            }

            /* --- Ossatura: umbrae structurales malae, mandibulae, temporis --- */
            Color bone_umb = col->cutis_umbra;

            /* Hollow malae: ellipsis lateralis sub malis (inter malam altam et mandibulam) */
            /* Centrum sinistra et dextra: sub oculis, paulo extra nasum */
            float hollow_amt = 0.0f;
            for (int side = 0; side < 2; side++) {
                float dir_side = (side == 0 ? -1.0f : 1.0f);
                float hcx = z->centrum_faciei.x + dir_side * z->lat_faciei * 0.42f;
                float hcy = z->oculi.y + z->alt_faciei * 0.32f;
                float hx = (q.x - hcx) / (z->lat_faciei * 0.18f + 1e-6f);
                float hy = (q.y - hcy) / (z->alt_faciei * 0.22f + 1e-6f);
                float hd = hx*hx + hy*hy;
                if (hd < 1.0f) {
                    hollow_amt = fmaxf(hollow_amt, (1.0f - hd) * 0.35f);
                }
            }
            if (hollow_amt > 0.01f) {
                Color hc = bone_umb;
                hc.a = 0.55f * hollow_amt;
                tabula_misce(t, x, y, hc);
            }

            /* Jaw shadow: linea sub mandibula arcuata — inter chin et auris */
            float jaw_amt = 0.0f;
            {
                float jcx = z->centrum_faciei.x;
                float jcy = z->centrum_faciei.y + z->alt_faciei * 0.78f;
                float jx = (q.x - jcx) / (z->lat_faciei * 0.85f + 1e-6f);
                float jy = (q.y - jcy) / (z->alt_faciei * 0.15f + 1e-6f);
                float jd = jx*jx + jy*jy;
                if (jd > 0.6f && jd < 1.2f) {
                    jaw_amt = (1.0f - fabsf(jd - 0.9f) * 3.5f) * 0.30f;
                    if (jaw_amt < 0.0f)
                        jaw_amt = 0.0f;
                }
            }
            if (jaw_amt > 0.01f) {
                Color jc = bone_umb;
                jc.a = 0.5f * jaw_amt;
                tabula_misce(t, x, y, jc);
            }

            /* Temple hollow: subtilis umbra supra angulos frontis */
            float temple_amt = 0.0f;
            for (int side = 0; side < 2; side++) {
                float dir_side = (side == 0 ? -1.0f : 1.0f);
                float tcx = z->centrum_faciei.x + dir_side * z->lat_faciei * 0.68f;
                float tcy = z->frons.y_frons + z->alt_faciei * 0.05f;
                float tx_n = (q.x - tcx) / (z->lat_faciei * 0.18f + 1e-6f);
                float ty_n = (q.y - tcy) / (z->alt_faciei * 0.15f + 1e-6f);
                float td = tx_n*tx_n + ty_n*ty_n;
                if (td < 1.0f) {
                    temple_amt = fmaxf(temple_amt, (1.0f - td) * 0.25f);
                }
            }
            if (temple_amt > 0.01f) {
                Color tc = bone_umb;
                tc.a = 0.45f * temple_amt;
                tabula_misce(t, x, y, tc);
            }

            /* Umbra sub oculis — subtilis "dark circles" pro aetate vel magnis fatigatione */
            float under_eye_amt = 0.0f;
            for (int side = 0; side < 2; side++) {
                vec2 eye_c = (side == 0) ? z->oculi.sin : z->oculi.dex;
                float ex = (q.x - eye_c.x) / (z->oculi.r_oculi_x * 1.4f + 1e-6f);
                float ey = (q.y - eye_c.y - z->oculi.r_oculi_y * 1.5f) / (z->oculi.r_oculi_y * 0.9f + 1e-6f);
                float ed = ex*ex + ey*ey;
                if (ed < 1.0f) {
                    under_eye_amt = fmaxf(under_eye_amt, (1.0f - ed) * (0.10f + 0.40f * p->aetas));
                }
            }
            if (under_eye_amt > 0.01f) {
                Color uc = bone_umb;
                uc.a = 0.6f * under_eye_amt;
                tabula_misce(t, x, y, uc);
            }
        }
    }

    /* Rima menti — sulcus verticalis in medio menti; vitanda si barba plena obtegit */
    if (p->rima_menti > 0.12f && p->modus_barbae != BARBA_PLENA && p->modus_barbae != BARBA_PROLIXA) {
        Color rim = col->cutis_umbra;
        rim.a = 0.35f + 0.45f * p->rima_menti;
        float mx = z->mentum.centrum.x;
        float my_top = z->os.centrum.y + z->os.alt * 2.0f;
        float my_bot = z->mentum.centrum.y + z->mentum.r * 0.55f;
        float len = (my_bot - my_top) * (0.45f + 0.55f * p->rima_menti);
        if (len > 2.0f) {
            tabula_pinge_lineam(t, v2(mx, my_top), v2(mx, my_top + len), 1.0f + 0.6f * p->rima_menti, rim);
        }
    }
}

/* ------------------------------------------------------------------ */
/* TEXTURA (pori + leves variationes)                                  */
/* ------------------------------------------------------------------ */

void redde_cutis_textura(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    (void)col;
    uint32_t semen = (uint32_t)(p->semen & 0xFFFFFFFFu) ^ 0x13371337u;
    int w = t->w, h = t->h;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            vec2 q = v2((float)x + 0.5f, (float)y + 0.5f);
            float d = sdf_contura_faciei(q, p, z);
            if (d > 0.0f)
                continue;
            float pn = fbm_s(q.x * 1.5f, q.y * 1.5f, 4, 0.5f, semen);
            float amp = 0.04f * p->visibilitas_pororum;
            float shade = pn * amp;
            int i = (y * w + x) * 4;
            t->pixels[i+0] = clampf(t->pixels[i+0] + shade, 0.0f, 1.0f);
            t->pixels[i+1] = clampf(t->pixels[i+1] + shade * 0.9f, 0.0f, 1.0f);
            t->pixels[i+2] = clampf(t->pixels[i+2] + shade * 0.8f, 0.0f, 1.0f);
        }
    }
}

/* ------------------------------------------------------------------ */
/* LENTIGINES                                                          */
/* ------------------------------------------------------------------ */

void redde_lentigines(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    if (p->densitas_lentiginorum < 0.01f)
        return;

    Sors s = sors_deriva(p->semen, 0x5EE5u);
    int N = 15 + (int)(80.0f * p->densitas_lentiginorum);   /* minus numerus, minor densitas */

    Color freck = color_obscurior(col->cutis, 0.35f);
    HSL h = color_ad_hsl(freck);
    /* Praeservat huam cutis nativam — non cogit omnes lentigines ad rubrum */
    h.s = saturatef(h.s * 0.8f + 0.15f);
    freck = hsl_ad_color(h, 0.7f);

    int placed = 0;
    int tries = 0;
    while (placed < N && tries < N * 10) {
        tries++;
        float x = sors_spatium(
            &s, z->centrum_faciei.x - z->lat_faciei,
            z->centrum_faciei.x + z->lat_faciei
        );
        /* Lentigines per totam faciem — ab fronte ad mentum — sed in genis et naso frequentiores */
        float y = sors_spatium(&s, z->frons.y_frons, z->os.centrum.y + z->alt_faciei * 0.10f);
        float d = sdf_contura_faciei(v2(x, y), p, z);
        if (d > -2.0f)
            continue;
        /* Distributio quasi-Poisson: habeat partem eiecta per strepitum FBM */
        if (fbm_s(x * 0.4f, y * 0.4f, 2, 0.5f, (uint32_t)(p->semen & 0xFFFFu)) < -0.2f)
            continue;

        float r = sors_spatium(&s, 0.4f, 1.0f);
        Color cc = freck;
        cc.a = 0.45f + 0.25f * sors_f32(&s);
        tabula_pinge_discum(t, v2(x, y), r, cc);
        placed++;
    }
}

/* ------------------------------------------------------------------ */
/* RUGAE                                                               */
/* ------------------------------------------------------------------ */

void redde_rugas(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    if (p->profunditas_rugarum < 0.15f)
        return;

    float amp = p->profunditas_rugarum;
    Color r_c = col->cutis_umbra;
    r_c.a = 0.4f * amp;

    /* Rugae in fronte (3 lineae horizontales ondulatae) */
    for (int i = 0; i < 3; i++) {
        float y = z->frons.y_frons + (float)i * 3.0f;
        int steps = 14;
        vec2 prev = v2(z->centrum_faciei.x - z->lat_faciei * 0.7f, y);
        for (int j = 1; j <= steps; j++) {
            float u = (float)j / (float)steps;
            float xx = z->centrum_faciei.x - z->lat_faciei * 0.7f + z->lat_faciei * 1.4f * u;
            float yy = y + sinf(u * 8.0f + (float)i * 2.0f) * 0.4f;
            tabula_pinge_lineam(t, prev, v2(xx, yy), 0.7f, r_c);
            prev = v2(xx, yy);
        }
    }

    /* "Crow's feet" anguli oculorum */
    for (int s = 0; s < 2; s++) {
        vec2 o = (s == 0) ? z->oculi.sin : z->oculi.dex;
        float dir = (s == 0 ? -1.0f : 1.0f);
        for (int i = 0; i < 3; i++) {
            vec2 a = v2(o.x + dir * z->oculi.r_oculi_x, o.y - 1.0f + (float)i * 1.5f);
            vec2 b = v2(a.x + dir * 5.0f, a.y + 1.5f);
            tabula_pinge_lineam(t, a, b, 0.6f, r_c);
        }
    }

    /* Sulcus nasolabialis */
    for (int s = 0; s < 2; s++) {
        float dir = (s == 0 ? -1.0f : 1.0f);
        vec2 a = v2(z->nasus.centrum.x + dir * z->nasus.lat * 1.05f, z->nasus.centrum.y + z->nasus.alt * 0.45f);
        vec2 b = v2(z->os.centrum.x + dir * z->os.lat * 1.1f, z->os.centrum.y + z->os.alt * 0.5f);
        tabula_pinge_lineam(t, a, b, 0.8f, r_c);
    }
}

/* ------------------------------------------------------------------ */
/* CICATRICES                                                          */
/* ------------------------------------------------------------------ */

void redde_cicatrices(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    if (p->numerus_cicatricum < 0.05f)
        return;
    Sors s = sors_deriva(p->semen, 0xCA1C21u);
    int N = 1 + (int)(p->numerus_cicatricum * 4.0f);
    Color cc = color_clariorem(col->cutis, 0.30f);
    cc.a = 0.8f;
    for (int i = 0; i < N; i++) {
        float cx = sors_spatium(
            &s, z->centrum_faciei.x - z->lat_faciei * 0.8f,
            z->centrum_faciei.x + z->lat_faciei * 0.8f
        );
        float cy = sors_spatium(
            &s, z->oculi.y - z->alt_faciei * 0.2f,
            z->centrum_faciei.y + z->alt_faciei * 0.5f
        );
        float ang = sors_spatium(&s, -1.5f, 1.5f);
        float len = sors_spatium(&s, 4.0f, 12.0f);
        vec2 a = v2(cx - cosf(ang) * len * 0.5f, cy - sinf(ang) * len * 0.5f);
        vec2 b = v2(cx + cosf(ang) * len * 0.5f, cy + sinf(ang) * len * 0.5f);
        tabula_pinge_lineam(t, a, b, 1.0f, cc);
    }
}

/* ------------------------------------------------------------------ */
/* DENTES                                                              */
/* ------------------------------------------------------------------ */

void redde_dentes(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    (void)col;
    if (p->modus_dentium == DENTES_NULLI)
        return;
    float sz = (p->magnitudo_dentis < 0.05f) ? 0.5f : p->magnitudo_dentis;
    Color alb = color4(0.95f, 0.93f, 0.85f, 1.0f);
    Color umb = color4(0.05f, 0.04f, 0.05f, 0.85f);
    Color flavus = color4(0.75f, 0.70f, 0.55f, 1.0f);  /* dens vetus */

    float os_cx = z->os.centrum.x;
    float os_lat = z->os.lat;
    /* labium superius subter (ubi dentes superi protrudunt) */
    float y_sup_edge = z->os.centrum.y - z->os.alt * 0.1f;
    /* labium inferius super (ubi dentes inferi protrudunt) */
    float y_inf_edge = z->os.centrum.y + z->os.alt * 1.3f;

    switch (p->modus_dentium) {
    case DENTES_CANINI_INFERI: {
            float dx = os_lat * 0.35f;
            float tw = 1.5f + 1.5f * sz;
            float th = 1.0f + 3.0f * sz;
            for (int s = 0; s < 2; s++) {
                float x = os_cx + (s == 0 ? -1.0f : 1.0f) * dx;
                tabula_pinge_lineam(t, v2(x, y_inf_edge - th), v2(x, y_inf_edge), tw, alb);
            }
            break;
        }
    case DENTES_CANINI_SUPERI: {
        /* Fangae superiōrēs descendentes super labium inferius */
            float dx = os_lat * 0.30f;
            float tw = 1.4f + 1.6f * sz;
            float th = 1.5f + 3.5f * sz;
            for (int s = 0; s < 2; s++) {
                float x = os_cx + (s == 0 ? -1.0f : 1.0f) * dx;
                tabula_pinge_lineam(t, v2(x, y_sup_edge), v2(x, y_sup_edge + th), tw, alb);
            /* puncta acuta extrema */
                tabula_pinge_discum(t, v2(x, y_sup_edge + th), tw * 0.45f, alb);
            }
            break;
        }
    case DENTES_SINGULARIS: {
        /* Unus dens asymmetricus — sinistra aut dextra per semen */
            Sors ss = sors_deriva(p->semen, 0xD001u);
            float side = (sors_f32(&ss) < 0.5f) ? -1.0f : 1.0f;
            float x = os_cx + side * os_lat * 0.25f;
            float tw = 1.8f + 2.0f * sz;
            float th = 1.5f + 3.5f * sz;
            tabula_pinge_lineam(t, v2(x, y_inf_edge - th), v2(x, y_inf_edge), tw, alb);
            break;
        }
    case DENTES_RUPTI: {
        /* Ordo interruptus — 5 positiones, aliquot vacuae aut rupti */
            Sors ss = sors_deriva(p->semen, 0xD002u);
            int N = 5;
            float tw = 1.6f + 1.0f * sz;
            float th = 1.2f + 2.2f * sz;
            for (int i = 0; i < N; i++) {
                float u = (float)i / (float)(N - 1);
                float x = os_cx + (u - 0.5f) * os_lat * 1.1f;
                float r = sors_f32(&ss);
                if (r < 0.35f)
                    continue;  /* lacuna */
                Color dc = (r < 0.60f) ? flavus : alb;
                float th_act = (r < 0.50f) ? th * 0.55f : th;  /* dens ruptus brevior */
                tabula_pinge_lineam(t, v2(x, y_inf_edge - th_act), v2(x, y_inf_edge), tw, dc);
            }
            break;
        }
    case DENTES_ORDO: {
        /* Ordo plenus — subrisus patens.  Solum si os apertum aut magnum. */
            float open = saturatef((p->altitudo_oris - 0.50f) * 2.0f);
            if (open < 0.15f && p->angulus_anguli_oris < 0.25f)
                return;  /* nihil ostendit nisi patens aut risus */
            int N = 7;
            float tw = 1.5f + 0.8f * sz;
            float th = 2.0f + 2.0f * sz;
        /* Background obscurus inter dentes */
            float ymid = (y_sup_edge + y_inf_edge) * 0.5f;
            tabula_pinge_lineam(
                t, v2(os_cx - os_lat * 0.7f, ymid), v2(os_cx + os_lat * 0.7f, ymid),
                th + 1.5f, umb
            );
            for (int i = 0; i < N; i++) {
                float u = (float)i / (float)(N - 1);
                float x = os_cx + (u - 0.5f) * os_lat * 1.3f;
                tabula_pinge_lineam(t, v2(x, ymid - th * 0.5f), v2(x, ymid + th * 0.5f), tw, alb);
            }
            break;
        }
    default: break;
    }
}

#include "lineamenta.h"
#include "color.h"
#include "sdf.h"
#include "sors.h"
#include "expressio.h"

PalettaFaciei paletta_computa(const FaciesParametra* p) {
    PalettaFaciei c;
    c.cutis        = color_cutis_basis(p->color_cutis, p->calor_cutis);
    /* Gens-specific shifts */
    if (p->gens == GENS_GIGANTUM) {
        /* virere tactus verdantis */
        HSL h = color_ad_hsl(c.cutis);
        h.h = 0.27f;
        h.s = mixf(h.s, 0.35f, 0.6f);
        c.cutis = hsl_ad_color(h, 1.0f);
    } else if (p->gens == GENS_LARVARUM) {
        c.cutis = color_desaturatus(c.cutis, 0.85f);
        c.cutis = color_obscurior(c.cutis, 0.15f);
        HSL h = color_ad_hsl(c.cutis);
        h.h = 0.6f;
        h.s = 0.05f;
        c.cutis = hsl_ad_color(h, 1.0f);
    } else if (p->gens == GENS_FURIARUM) {
        HSL h = color_ad_hsl(c.cutis);
        h.h = 0.00f;  /* rubrum */
        h.s = mixf(h.s, 0.7f, 0.5f);
        h.l = mixf(h.l, 0.35f, 0.3f);
        c.cutis = hsl_ad_color(h, 1.0f);
    } else if (
        p->gens == GENS_NYMPHARUM
        && (p->semen & 0xFu) < 5u
    ) {
        /* Nymphae occasionales caeruleae/aquaticae (~31%) */
        HSL h = color_ad_hsl(c.cutis);
        h.h = mixf(0.52f, 0.58f, (float)((p->semen >> 4) & 0xFu) / 15.0f);
        h.s = mixf(h.s, 0.45f, 0.6f);
        h.l = mixf(h.l, 0.65f, 0.3f);
        c.cutis = hsl_ad_color(h, 1.0f);
    } else if (
        p->gens == GENS_HUMANA
        && (p->semen & 0x3Fu) < 2u
    ) {
        /* Rara tinctus caeruleus etiam inter humanos (~3%) */
        HSL h = color_ad_hsl(c.cutis);
        h.h = 0.57f;
        h.s = mixf(h.s, 0.30f, 0.4f);
        h.l = mixf(h.l, 0.55f, 0.15f);
        c.cutis = hsl_ad_color(h, 1.0f);
    }
    c.cutis_umbra  = color_obscurior(c.cutis, 0.35f);
    c.cutis_lux    = color_clariorem(c.cutis, 0.20f);
    /* Luminositas iridis: exotici (hūa extra regionem calidam) clariores ut pop
     * in tesserīs parvīs; naturales servant profunditatem mediam. */
    {
        /* Distantia hūae a regione calida (brunneus ~0.08) */
        float dh = p->color_iridis_h - 0.08f;
        if (dh < 0.0f)
            dh = -dh;
        if (dh > 0.5f)
            dh = 1.0f - dh;
        float iris_l;
        if (p->color_iridis_s < 0.12f)
            iris_l = 0.72f;  /* cani/albi clarissimi */
        else if (dh > 0.18f)
            iris_l = 0.52f;  /* exotici clariores */
        else
            iris_l = 0.45f - 0.12f * p->color_iridis_s;
        c.iridis = color_iridis_base(p->color_iridis_h, p->color_iridis_s, iris_l);
    }
    if (p->gens == GENS_FURIARUM)
        c.iridis = color4(1.0f, 0.85f, 0.0f, 1.0f);
    if (p->gens == GENS_LARVARUM)
        c.iridis = color4(0.70f, 0.75f, 0.70f, 1.0f);
    c.labia        = color_labiorum(c.cutis, 0.7f);
    c.sclera       = color_sclera(p->venae_sclerae + (p->gens == GENS_LARVARUM ? 0.3f : 0.0f));
    c.os_interior  = color_obscurior(c.labia, 0.55f);
    c.comae        = color_comae(p->color_comae_h, p->color_comae_s, p->color_comae_v, p->canitudo);
    c.comae_obscura = color_obscurior(c.comae, 0.30f);
    c.supercilia   = color_obscurior(c.comae, 0.20f);
    c.lineae       = color_obscurior(c.cutis_umbra, 0.6f);
    return c;
}

/* ------------------------------------------------------------------ */
/* AURES                                                              */
/* ------------------------------------------------------------------ */

typedef struct {
    vec2 c;
    float lat, alt;
    float acies;   /* 0=rotunda, 1=elfica */
    int   latus;   /* -1 sinistra, +1 dextra */
} AurisCtx;

static float sdf_auris(vec2 p, void* ctx_) {
    AurisCtx* ctx = (AurisCtx*) ctx_;
    vec2 q = v2_sub(p, ctx->c);
    /* ellipsis + punctum acutum pro elforum */
    float d = sdf_ellipsis(q, ctx->lat, ctx->alt);
    if (ctx->acies > 0.01f) {
        /* triangulus superne */
        vec2 q2 = v2(
            q.x + (float)ctx->latus * ctx->lat * 0.3f,
            q.y + ctx->alt * (0.9f + 0.4f * ctx->acies)
        );
        float tri = sdf_triangulum_aeq(q2, ctx->alt * (0.5f + 0.4f * ctx->acies));
        d = sdf_unio_lenis(d, tri, 2.0f);
    }
    return d;
}

void redde_aures(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    /* Duas aures — solum lateralis umbra et curva interior */
    for (int s = 0; s < 2; s++) {
        AurisCtx ctx;
        ctx.c     = (s == 0) ? z->aures.sin : z->aures.dex;
        ctx.lat   = z->aures.lat;
        ctx.alt   = z->aures.alt;
        ctx.acies = z->aures.acies;
        ctx.latus = (s == 0) ? -1 : 1;

        float rr = fmaxf(ctx.lat, ctx.alt) * (1.0f + 0.5f * ctx.acies) + 4.0f;

        /* Corpus auris */
        tabula_pinge_sdf(t, ctx.c, rr, sdf_auris, &ctx, col->cutis);

        /* Umbra interior — ellipsis minor concava */
        vec2 c2 = v2(ctx.c.x - (float)ctx.latus * ctx.lat * 0.2f, ctx.c.y + ctx.alt * 0.05f);
        AurisCtx inn = ctx;
        inn.c = c2;
        inn.lat = ctx.lat * 0.55f;
        inn.alt = ctx.alt * 0.6f;
        inn.acies = 0.0f;
        tabula_pinge_sdf(t, c2, fmaxf(inn.lat, inn.alt) + 2.0f, sdf_auris, &inn, col->cutis_umbra);

        /* Lobulus — circulus inferus, solum si non-elfiche */
        if (ctx.acies < 0.5f) {
            float lobo_r = ctx.lat * (0.3f + 0.25f * p->magnitudo_lobi);
            vec2  lobo_c = v2(ctx.c.x, ctx.c.y + ctx.alt * 0.8f);
            tabula_pinge_discum(t, lobo_c, lobo_r, col->cutis);
        }
    }
}

/* ------------------------------------------------------------------ */
/* OCULI                                                              */
/* ------------------------------------------------------------------ */

typedef struct {
    vec2 c;
    float rx, ry;
    float inclinatio;
    float apertio;      /* 0..1 */
    float pondus_pal;   /* 0..1 — palpebra gravior */
} OculusCtx;

static float sdf_oculus_forma(vec2 p, void* ctx_) {
    OculusCtx* ctx = (OculusCtx*) ctx_;
    vec2 q = v2_rotatus(v2_sub(p, ctx->c), -ctx->inclinatio);
    /* lens-like amygdala */
    float ry_eff = ctx->ry * ctx->apertio;
    return sdf_lens(q, ctx->rx, ctx->rx * 0.9f + ry_eff * 0.9f);
}

static float sdf_iridis(vec2 p, void* ctx_) {
    OculusCtx* ctx = (OculusCtx*) ctx_;
    vec2 q = v2_sub(p, ctx->c);
    return sdf_circulus(q, ctx->rx * 0.6f);
}

void redde_oculos(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    float nictus = expressio_nictus(p->tempus);
    /* Color iridis per latus — heterochromia usat color_iridis2 prō dextro */
    Color iridis_sin = col->iridis;
    Color iridis_dex;
    if (p->heterochromia > 0.5f) {
        float l2 = 0.55f - 0.15f * p->color_iridis2_s;
        iridis_dex = color_iridis_base(p->color_iridis2_h, p->color_iridis2_s, l2);
    } else {
        iridis_dex = col->iridis;
    }
    if (p->gens == GENS_FURIARUM)
        iridis_dex = iridis_sin;  /* oculi Furiis identici */
    if (p->gens == GENS_LARVARUM)
        iridis_dex = iridis_sin;

    for (int s = 0; s < 2; s++) {
        Color iridis_c = (s == 0) ? iridis_sin : iridis_dex;
        OculusCtx ctx;
        ctx.c = (s == 0) ? z->oculi.sin : z->oculi.dex;
        /* Asymmetria: signata — sin maior si positiva; adde parvum offset y */
        float asym = p->oculi_asymmetria * (s == 0 ? 1.0f : -1.0f);
        ctx.rx = z->oculi.r_oculi_x * (1.0f + asym * 0.18f);
        ctx.ry = z->oculi.r_oculi_y * (1.0f + asym * 0.14f);
        ctx.c.y += asym * z->oculi.r_oculi_y * 0.25f;
        ctx.inclinatio = (s == 0 ? -1.0f : 1.0f) * z->oculi.inclinatio;
        /* pondus palpebrae clausit oculum leviter */
        float apertio = saturatef((1.0f - 0.7f * p->pondus_palpebrae) * nictus);
        ctx.apertio = apertio;
        ctx.pondus_pal = p->pondus_palpebrae;

        if (apertio < 0.05f) {
            /* oculus clausus — linea obscura */
            vec2 a = v2(ctx.c.x - ctx.rx * 0.95f, ctx.c.y);
            vec2 b = v2(ctx.c.x + ctx.rx * 0.95f, ctx.c.y);
            tabula_pinge_lineam(t, a, b, 1.5f, col->lineae);
            continue;
        }

        /* Sclera */
        float raggio = fmaxf(ctx.rx, ctx.ry) + 2.0f;
        tabula_pinge_sdf(t, ctx.c, raggio, sdf_oculus_forma, &ctx, col->sclera);

        /* Iris — discus sub palpebra superiore (colore per latus) */
        OculusCtx inn = ctx;
        inn.rx = z->oculi.r_iridis;
        inn.ry = z->oculi.r_iridis;
        tabula_pinge_sdf(t, ctx.c, inn.rx + 2.0f, sdf_iridis, &inn, iridis_c);

        /* Radius iris actualis (sdf_iridis adhibet rx * 0.6) */
        float iris_r = z->oculi.r_iridis * 0.60f;

        /* Striae radiales — lineae breves alternantes clariores et obscuriores */
        Sors iss = sors_deriva(p->semen, 0xE1E1u ^ (uint64_t)(s + 1));
        int n_striae = 14 + (int)(sors_f32(&iss) * 8.0f);
        Color stria_clara = color_clariorem(iridis_c, 0.35f);
        Color stria_obsc  = color_obscurior(iridis_c, 0.40f);
        for (int i = 0; i < n_striae; i++) {
            float ang = ((float)i + sors_spatium(&iss, -0.25f, 0.25f))
                / (float)n_striae * PORTRAIT_TAU;
            float r_in  = iris_r * sors_spatium(&iss, 0.28f, 0.40f);
            float r_out = iris_r * sors_spatium(&iss, 0.82f, 0.98f);
            vec2 a = v2(ctx.c.x + cosf(ang) * r_in,  ctx.c.y + sinf(ang) * r_in);
            vec2 b = v2(ctx.c.x + cosf(ang) * r_out, ctx.c.y + sinf(ang) * r_out);
            Color sc = (i & 1) ? stria_clara : stria_obsc;
            sc.a = 0.55f + 0.20f * sors_f32(&iss);
            tabula_pinge_lineam(t, a, b, 0.7f, sc);
        }

        /* Limbus — annulus obscurus ad marginem iridis */
        Color limbus = color_obscurior(iridis_c, 0.75f);
        tabula_pinge_discum(t, ctx.c, iris_r + 0.8f, limbus);
        tabula_pinge_discum(t, ctx.c, iris_r - 0.3f, iridis_c);
        /* Re-pingit strias super iris normalem ne per limbus obtegerentur */
        Sors iss2 = sors_deriva(p->semen, 0xE1E1u ^ (uint64_t)(s + 1));
        for (int i = 0; i < n_striae; i++) {
            float ang = ((float)i + sors_spatium(&iss2, -0.25f, 0.25f))
                / (float)n_striae * PORTRAIT_TAU;
            float r_in  = iris_r * sors_spatium(&iss2, 0.28f, 0.40f);
            float r_out = iris_r * sors_spatium(&iss2, 0.78f, 0.92f);
            vec2 a = v2(ctx.c.x + cosf(ang) * r_in,  ctx.c.y + sinf(ang) * r_in);
            vec2 b = v2(ctx.c.x + cosf(ang) * r_out, ctx.c.y + sinf(ang) * r_out);
            Color sc = (i & 1) ? stria_clara : stria_obsc;
            sc.a = 0.45f + 0.15f * sors_f32(&iss2);
            tabula_pinge_lineam(t, a, b, 0.6f, sc);
        }

        /* Pupilla — circulus niger parva */
        Color pup = color4(0.03f, 0.03f, 0.04f, 1.0f);
        if (p->gens == GENS_FURIARUM) {
            /* pupilla fissa felina — capsula verticalis */
            vec2 a = v2(ctx.c.x, ctx.c.y - iris_r * 0.85f);
            vec2 b = v2(ctx.c.x, ctx.c.y + iris_r * 0.85f);
            tabula_pinge_lineam(t, a, b, 1.8f, pup);
        } else {
            tabula_pinge_discum(t, ctx.c, iris_r * 0.42f, pup);
        }

        /* Nitor — puncta alba (2 puncta in stylo cartoon) */
        Color albus = color4(1.0f, 1.0f, 1.0f, 0.85f);
        vec2 nitor1 = v2(ctx.c.x - inn.rx * 0.28f, ctx.c.y - inn.rx * 0.28f);
        vec2 nitor2 = v2(ctx.c.x - inn.rx * 0.10f, ctx.c.y + inn.rx * 0.08f);
        tabula_pinge_discum(t, nitor1, inn.rx * 0.20f, albus);
        tabula_pinge_discum(t, nitor2, inn.rx * 0.08f, color4(1, 1, 1, 0.55f));

        /* Cilia — lineae breves supra */
        int n_cil = 4;
        for (int i = 0; i < n_cil; i++) {
            float u = (i + 0.5f) / (float)n_cil;
            float cx = ctx.c.x - ctx.rx * 0.8f + ctx.rx * 1.6f * u;
            float cy = ctx.c.y - ctx.ry * 0.9f;
            vec2 a = v2(cx, cy);
            vec2 b = v2(cx + (s == 0 ? -1.0f : 1.0f) * 1.3f, cy - 2.0f);
            tabula_pinge_lineam(t, a, b, 0.9f, col->lineae);
        }

        /* Palpebra superior — linea lata delineata */
        {
            vec2 a = v2(ctx.c.x - ctx.rx, ctx.c.y);
            vec2 b = v2(ctx.c.x + ctx.rx, ctx.c.y);
            vec2 ma = v2_rotatus(v2_sub(a, ctx.c), ctx.inclinatio);
            vec2 mb = v2_rotatus(v2_sub(b, ctx.c), ctx.inclinatio);
            ma = v2_add(ma, ctx.c);
            mb = v2_add(mb, ctx.c);
            ma.y -= ctx.ry * 0.6f * apertio;
            mb.y -= ctx.ry * 0.6f * apertio;
            /* corrigor approximatus — sufficit pro cartoon */
            tabula_pinge_lineam(t, ma, mb, 1.2f, col->lineae);
        }

        /* Plica epicanthica — velatio parva angulum internum tegens.
         * s == 0 est sinister (dir_in = +1 sc. ad medium); s == 1 est dexter (dir_in = -1). */
        if (p->plica_epicanthica > 0.10f) {
            float dir_in = (s == 0) ? 1.0f : -1.0f;
            float ext = p->plica_epicanthica;
            vec2 corner = v2(ctx.c.x - dir_in * ctx.rx * 0.95f, ctx.c.y);
            vec2 over = v2(
                corner.x + dir_in * ctx.rx * 0.35f * ext,
                corner.y - ctx.ry * 0.20f * ext
            );
            vec2 down = v2(
                corner.x + dir_in * ctx.rx * 0.10f * ext,
                corner.y + ctx.ry * 0.55f * ext
            );
            Color plica_c = col->lineae;
            plica_c.a = 0.55f + 0.35f * ext;
            int Np = 6;
            vec2 prev = over;
            for (int i = 1; i <= Np; i++) {
                float u = (float)i / (float)Np;
                float om = 1.0f - u;
                vec2 pt = v2(
                    om*om*over.x + 2*om*u*corner.x + u*u*down.x,
                    om*om*over.y + 2*om*u*corner.y + u*u*down.y
                );
                tabula_pinge_lineam(t, prev, pt, 0.9f, plica_c);
                prev = pt;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* SUPERCILIA                                                         */
/* ------------------------------------------------------------------ */

void redde_supercilia(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    for (int s = 0; s < 2; s++) {
        vec2 centrum = (s == 0) ? z->supercilia_sin.centrum : z->supercilia_dex.centrum;
        float lat = z->supercilia_sin.lat;
        float crass = 0.8f + 2.2f * p->crassitudo_supercilii;
        /* Arcus supercilii: curva Bezier per 3 puncta.
         *  arcus_supercilii [0..1]: 0=planus, 1=arcus altus */
        float arch = (p->arcus_supercilii - 0.5f) * (crass * 2.0f);
        vec2 a = v2(centrum.x - lat, centrum.y - arch * 0.2f);
        vec2 b = v2(centrum.x,        centrum.y - arch);
        vec2 c = v2(centrum.x + lat, centrum.y - arch * 0.2f);
        /* Pingimus per segmenta multiplices — approximatio Bezier */
        int N = 8;
        vec2 prev = a;
        for (int i = 1; i <= N; i++) {
            float u = (float)i / (float)N;
            /* Bezier quad */
            float om = 1.0f - u;
            vec2 pt = v2(
                om*om*a.x + 2*om*u*b.x + u*u*c.x,
                om*om*a.y + 2*om*u*b.y + u*u*c.y
            );
            tabula_pinge_lineam(t, prev, pt, crass, col->supercilia);
            prev = pt;
        }
    }
}

/* ------------------------------------------------------------------ */
/* NASUS                                                              */
/* ------------------------------------------------------------------ */

void redde_nasum(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    vec2 c = z->nasus.centrum;
    float lat = z->nasus.lat;
    float alt = z->nasus.alt;

    /* Umbrae subtiles in lateribus pontis.
     * altitudo_pontis [0..1]: 0 = pons planus (nullum contrast), 1 = pons altus cum
     * umbra densa et nitore medio. */
    float ponte_lat = z->nasus.pontis_lat;
    float pons_alt  = p->altitudo_pontis;
    Color umb = col->cutis_umbra;
    umb.a = 0.20f + 0.45f * pons_alt;

    /* Linea dextra umbrata */
    vec2 a1 = v2(c.x + ponte_lat * 0.8f, c.y - alt * 0.4f);
    vec2 b1 = v2(c.x + ponte_lat * 0.9f, c.y + alt * 0.3f);
    tabula_pinge_lineam(t, a1, b1, 1.0f + 0.8f * pons_alt, umb);

    /* Nitor medianus super pontem — tantum si pons altus */
    if (pons_alt > 0.35f) {
        Color lux_pons = col->cutis_lux;
        lux_pons.a = 0.25f * (pons_alt - 0.35f) / 0.65f;
        vec2 la = v2(c.x, c.y - alt * 0.45f);
        vec2 lb = v2(c.x, c.y + alt * 0.10f);
        tabula_pinge_lineam(t, la, lb, 0.9f, lux_pons);
    }

    /* Apex nasi — ellipsis parva umbrata.  Formam modificant forma_apicis. */
    float apex_rx = lat * (0.5f + 0.3f * p->forma_apicis);
    float apex_ry = lat * (0.35f + 0.15f * p->forma_apicis);
    vec2 apex_c = v2(c.x, c.y + alt * 0.35f);
    /* subtilis gradient */
    tabula_pinge_discum(t, apex_c, apex_rx, umb);
    Color luxe = col->cutis_lux;
    luxe.a = 0.3f;
    tabula_pinge_discum(t, v2(apex_c.x, apex_c.y - apex_ry * 0.3f), apex_rx * 0.5f, luxe);

    /* Nares — duae parvae puncta obscurae */
    Color naris = color_obscurior(col->cutis, 0.55f);
    naris.a = 0.85f;
    float nares_dx = lat * (0.35f + 0.25f * p->dilatatio_narium);
    float nares_r  = lat * (0.12f + 0.10f * p->dilatatio_narium);
    tabula_pinge_discum(t, v2(c.x - nares_dx, c.y + alt * 0.42f), nares_r, naris);
    tabula_pinge_discum(t, v2(c.x + nares_dx, c.y + alt * 0.42f), nares_r, naris);

    /* Philtrum — sulcus inter nasum et labium superius.
     * Duae lineae umbratae verticales, profunditas per profunditas_philtri. */
    if (p->profunditas_philtri > 0.08f) {
        Color phil = col->cutis_umbra;
        phil.a = 0.25f + 0.45f * p->profunditas_philtri;
        float phil_dx = lat * (0.12f + 0.04f * p->profunditas_philtri);
        float y_top = c.y + alt * 0.55f;
        float y_bot = z->os.centrum.y - z->os.alt * 1.2f;
        if (y_bot > y_top) {
            tabula_pinge_lineam(t, v2(c.x - phil_dx, y_top), v2(c.x - phil_dx, y_bot), 0.7f, phil);
            tabula_pinge_lineam(t, v2(c.x + phil_dx, y_top), v2(c.x + phil_dx, y_bot), 0.7f, phil);
        }
    }
}

/* ------------------------------------------------------------------ */
/* OS                                                                 */
/* ------------------------------------------------------------------ */

/* Helper: evalua Bezier quadraticus in u ∈ [0,1] */
static inline vec2 bez2(vec2 a, vec2 b, vec2 c, float u) {
    float om = 1.0f - u;
    return v2(
        om*om*a.x + 2*om*u*b.x + u*u*c.x,
        om*om*a.y + 2*om*u*b.y + u*u*c.y
    );
}

void redde_os(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    vec2  c   = z->os.centrum;
    float lat = z->os.lat;
    float alt = z->os.alt;
    float ang = p->angulus_anguli_oris;

    /* Dimensio labiorum in pixeliis absolutis */
    float t_sup = 0.8f + 1.8f * p->crassitudo_labii_sup;   /* 0.8..2.6 */
    float t_inf = 1.2f + 2.6f * p->crassitudo_labii_inf;   /* 1.2..3.8 */

    /* Apertura oris — si altitudo ora > 0.55 */
    float open = saturatef((p->altitudo_oris - 0.55f) * 2.5f);

    /* Puncta angularia (anguli oris flectuntur sursum/deorsum ex ang) */
    vec2 lc = v2(c.x - lat,      c.y - ang * 2.0f);
    vec2 rc = v2(c.x + lat,      c.y - ang * 2.0f);

    /* Linea oris — curva medans per medium cum modificatione apertionis */
    vec2 ml_mid = v2(c.x, c.y + open * alt * 0.25f);

    /* Summa labii superioris — cum arcu cupidinis duobus apicibus et fossa */
    float cup = p->arcus_cupidinis * 1.0f;
    vec2 sup_apex_l = v2(c.x - lat * 0.30f, c.y - t_sup - 0.6f * cup);
    vec2 sup_apex_r = v2(c.x + lat * 0.30f, c.y - t_sup - 0.6f * cup);
    vec2 sup_fossa  = v2(c.x,               c.y - t_sup * 0.55f);
    /* Via angularia — curvae in cauda */
    vec2 sup_ctrl_l = v2(c.x - lat * 0.70f, c.y - t_sup * 0.5f);
    vec2 sup_ctrl_r = v2(c.x + lat * 0.70f, c.y - t_sup * 0.5f);

    /* Summa labii inferioris — plenior, cum suo puncto medali descendente */
    vec2 inf_mid   = v2(c.x, c.y + t_inf + open * alt * 0.5f);
    vec2 inf_ctrl_l = v2(c.x - lat * 0.65f, c.y + t_inf * 0.35f);
    vec2 inf_ctrl_r = v2(c.x + lat * 0.65f, c.y + t_inf * 0.35f);

    /* Pingimus labium superius ut ellipsin lenem via verticales segmenta */
    int N = 22;
    for (int i = 0; i < N; i++) {
        float u = (float)i / (float)(N - 1);
        /* Summa (curva top): quattuor puncta via duorum Bezier connexorum */
        vec2 top;
        if (u < 0.5f) {
            float uu = u * 2.0f;
            /* lc -> sup_ctrl_l -> sup_apex_l -> fossa */
            vec2 m1 = bez2(lc, sup_ctrl_l, sup_apex_l, uu);
            vec2 m2 = bez2(sup_ctrl_l, sup_apex_l, sup_fossa, uu);
            top = v2(mixf(m1.x, m2.x, uu), mixf(m1.y, m2.y, uu));
        } else {
            float uu = (u - 0.5f) * 2.0f;
            /* fossa -> sup_apex_r -> sup_ctrl_r -> rc */
            vec2 m1 = bez2(sup_fossa, sup_apex_r, sup_ctrl_r, uu);
            vec2 m2 = bez2(sup_apex_r, sup_ctrl_r, rc, uu);
            top = v2(mixf(m1.x, m2.x, uu), mixf(m1.y, m2.y, uu));
        }
        /* Bot (linea oris): lc -> ml_mid -> rc */
        vec2 bot = bez2(lc, ml_mid, rc, u);
        tabula_pinge_lineam(t, top, bot, 1.8f, col->labia);
    }

    /* Labium inferius */
    for (int i = 0; i < N; i++) {
        float u = (float)i / (float)(N - 1);
        vec2 top = bez2(lc, ml_mid, rc, u);
        /* bot: lc -> inf_ctrl_l -> inf_mid -> inf_ctrl_r -> rc (duos Bezier) */
        vec2 bot;
        if (u < 0.5f) {
            float uu = u * 2.0f;
            bot = bez2(lc, inf_ctrl_l, inf_mid, uu);
        } else {
            float uu = (u - 0.5f) * 2.0f;
            bot = bez2(inf_mid, inf_ctrl_r, rc, uu);
        }
        tabula_pinge_lineam(t, top, bot, 1.8f, col->labia);
    }

    /* Interiorem oris si apertum */
    if (open > 0.05f) {
        /* Inter labium superius et inferius */
        float ir = lat * 0.70f;
        float ih = open * alt * 0.35f + 1.0f;
        vec2 ic = v2(c.x, c.y + open * alt * 0.20f);
        Color dark = col->os_interior;
        /* Ellipsis pro interiore */
        for (int i = 0; i < 16; i++) {
            float u = (float)i / 15.0f;
            vec2 pa = v2(ic.x - ir + 2.0f * ir * u, ic.y - ih);
            vec2 pb = v2(ic.x - ir + 2.0f * ir * u, ic.y + ih);
            tabula_pinge_lineam(t, pa, pb, 2.0f, dark);
        }
    }

    /* Linea oris (obscurior) — cum apertura mitigata */
    {
        Color line = col->lineae;
        line.a = saturatef(1.0f - open * 0.8f);
        int Nm = 18;
        vec2 prev = lc;
        for (int i = 1; i <= Nm; i++) {
            float u = (float)i / (float)Nm;
            vec2 pt = bez2(lc, ml_mid, rc, u);
            tabula_pinge_lineam(t, prev, pt, 0.8f, line);
            prev = pt;
        }
    }
}

/* ------------------------------------------------------------------ */
/* CORNUA (pro daemonum / orcorum)                                    */
/* ------------------------------------------------------------------ */

void redde_cornua(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
) {
    (void)col;
    if (p->magnitudo_cornu < 0.05f)
        return;
    float amount = p->magnitudo_cornu;
    /* Cornua curvata emergit ex fronte lateribus */
    float dist = z->lat_faciei * 0.75f;
    float y    = z->frons.y_frons - 4.0f;
    float len  = z->alt_faciei * (0.2f + 0.3f * amount);
    Color cornu_c = color4(0.25f, 0.15f, 0.10f, 1.0f);
    for (int s = 0; s < 2; s++) {
        float dx = (s == 0 ? -1.0f : 1.0f) * dist;
        vec2 base = v2(z->centrum_faciei.x + dx, y);
        int N = 8;
        vec2 prev = base;
        for (int i = 1; i <= N; i++) {
            float u = (float)i / (float)N;
            float cx = base.x + (s == 0 ? -1.0f : 1.0f) * len * 0.5f * u;
            float cy = base.y - len * u * (1.0f - 0.3f * u);
            vec2 pt = v2(cx, cy);
            tabula_pinge_lineam(t, prev, pt, 2.8f * (1.0f - 0.7f * u), cornu_c);
            prev = pt;
        }
    }
}

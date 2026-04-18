#include "expressio.h"

/* Unusquisque constantis expressio est delta ex neutro. Expressio est aduectata
 * super FaciesParametra per `pondus` in [0..1]. */
const Expressio EXPR_NEUTRUM = { 0 };

const Expressio EXPR_LAETUM = {
    /* Supercilium levantur leviter externe */
        .supercilium_leva_int =  0.05f, .supercilium_dex_int =  0.05f,
        .supercilium_leva_ext =  0.10f, .supercilium_dex_ext =  0.10f,
        .apertio_oculi_sin    = -0.15f, .apertio_oculi_dex   = -0.15f,
        .contractio_oculi_sin =  0.20f, .contractio_oculi_dex =  0.20f,
        .apertio_oris         =  0.25f,
        .risus                =  0.85f,
        .latitudo_oris        =  0.20f,
        .tractio_anguli_sin   =  0.70f, .tractio_anguli_dex  =  0.70f,
        .elevatio_genae       =  0.50f,
};

const Expressio EXPR_TRISTE = {
    .supercilium_leva_int =  0.35f, .supercilium_dex_int =  0.35f,
    .supercilium_leva_ext = -0.20f, .supercilium_dex_ext = -0.20f,
    .contractio_supercilii =  0.30f,
    .apertio_oculi_sin    = -0.10f, .apertio_oculi_dex   = -0.10f,
    .risus                = -0.75f,
    .tractio_anguli_sin   = -0.60f, .tractio_anguli_dex  = -0.60f,
    .elevatio_menti       =  0.20f,
};

const Expressio EXPR_IRATUM = {
    .supercilium_leva_int = -0.40f, .supercilium_dex_int = -0.40f,
    .supercilium_leva_ext =  0.15f, .supercilium_dex_ext =  0.15f,
    .contractio_supercilii =  0.70f,
    .apertio_oculi_sin    = -0.05f, .apertio_oculi_dex   = -0.05f,
    .contractio_oculi_sin =  0.15f, .contractio_oculi_dex =  0.15f,
    .risus                = -0.40f,
    .tractio_anguli_sin   = -0.30f, .tractio_anguli_dex  = -0.30f,
    .dilatatio_narium     =  0.50f,
};

const Expressio EXPR_MIRANS = {
    .supercilium_leva_int =  0.60f, .supercilium_dex_int =  0.60f,
    .supercilium_leva_ext =  0.60f, .supercilium_dex_ext =  0.60f,
    .apertio_oculi_sin    =  0.50f, .apertio_oculi_dex   =  0.50f,
    .apertio_oris         =  0.60f,
    .descensus_maxillae   =  0.60f,
};

const Expressio EXPR_TIMIDUM = {
    .supercilium_leva_int =  0.50f, .supercilium_dex_int =  0.50f,
    .supercilium_leva_ext =  0.20f, .supercilium_dex_ext =  0.20f,
    .contractio_supercilii =  0.40f,
    .apertio_oculi_sin    =  0.65f, .apertio_oculi_dex   =  0.65f,
    .apertio_oris         =  0.30f,
    .latitudo_oris        = -0.30f,
    .tractio_anguli_sin   = -0.30f, .tractio_anguli_dex  = -0.30f,
    .descensus_maxillae   =  0.20f,
};

const Expressio EXPR_FASTIDIOSUM = {
    .supercilium_leva_int = -0.10f, .supercilium_dex_int = -0.10f,
    .apertio_oculi_sin    = -0.15f, .apertio_oculi_dex   = -0.15f,
    .contractio_oculi_sin =  0.10f, .contractio_oculi_dex =  0.10f,
    .dilatatio_narium     =  0.30f,
    .risus                = -0.35f,
    .tractio_anguli_sin   = -0.20f, .tractio_anguli_dex  = -0.20f,
    .elevatio_menti       =  0.10f,
};

const Expressio EXPR_CONTEMPTUM = {
    /* Asymmetricum — unum latus ore sursum */
        .supercilium_leva_int = -0.05f, .supercilium_dex_int =  0.10f,
        .tractio_anguli_sin   = -0.10f, .tractio_anguli_dex  =  0.55f,
        .dilatatio_narium     =  0.15f,
        .apertio_oculi_sin    = -0.05f, .apertio_oculi_dex   = -0.05f,
        .contractio_oculi_dex =  0.15f,
};

Expressio expressio_misce(Expressio a, Expressio b, float t) {
    Expressio r;
    const float* pa = (const float*)&a;
    const float* pb = (const float*)&b;
    float*       pr = (float*)      &r;
    size_t n = sizeof(Expressio) / sizeof(float);
    for (size_t i = 0; i < n; i++)
        pr[i] = pa[i] + (pb[i] - pa[i]) * t;
    return r;
}

void facies_applica_expressionem(FaciesParametra* p, const Expressio* e, float pondus) {
    if (!p || !e)
        return;
    float w = saturatef(pondus);

    /* Supercilium */
    p->altitudo_supercilii = saturatef(
        p->altitudo_supercilii
        + w * 0.25f * (
            (
                e->supercilium_leva_int + e->supercilium_leva_ext
                + e->supercilium_dex_int + e->supercilium_dex_ext
            ) * 0.25f
        )
    );
    p->intervallum_supercilii = saturatef(p->intervallum_supercilii - w * 0.25f * e->contractio_supercilii);

    /* Oculi */
    float ap_sin = p->magnitudo_oculi * (1.0f + w * 0.5f * e->apertio_oculi_sin);
    float ap_dex = p->magnitudo_oculi * (1.0f + w * 0.5f * e->apertio_oculi_dex);
    p->magnitudo_oculi = saturatef(0.5f * (ap_sin + ap_dex));
    p->pondus_palpebrae = saturatef(
        p->pondus_palpebrae
        + w * 0.5f * 0.5f * (e->contractio_oculi_sin + e->contractio_oculi_dex)
    );

    /* Os */
    p->altitudo_oris   = saturatef(p->altitudo_oris   + w * 0.45f * e->apertio_oris);
    p->latitudo_oris   = saturatef(p->latitudo_oris   + w * 0.30f * e->latitudo_oris);
    p->angulus_anguli_oris = clampf(
        p->angulus_anguli_oris
        + w * 0.5f * (e->risus + 0.5f * (e->tractio_anguli_sin + e->tractio_anguli_dex)), -1.5f, 1.5f
    );
    p->crassitudo_labii_sup = saturatef(p->crassitudo_labii_sup - w * 0.10f * fabsf(e->risus));
    p->crassitudo_labii_inf = saturatef(p->crassitudo_labii_inf + w * 0.05f * e->apertio_oris);

    p->dilatatio_narium = saturatef(p->dilatatio_narium + w * 0.4f * e->dilatatio_narium);
}

/* Nictus: duo eventus discreti in tempus [0,1). Brevis: ~0.04 longitudine. */
float expressio_nictus(float tempus) {
    tempus -= floorf(tempus);
    float centrum1 = 0.10f;
    float centrum2 = 0.60f;
    float dur = 0.04f;

    float d1 = fabsf(tempus - centrum1);
    if (d1 > 0.5f)
        d1 = 1.0f - d1;
    float d2 = fabsf(tempus - centrum2);
    if (d2 > 0.5f)
        d2 = 1.0f - d2;

    float c1 = expf(-(d1 * d1) / (dur * dur * 0.2f));
    float c2 = expf(-(d2 * d2) / (dur * dur * 0.2f));
    float cc = fmaxf(c1, c2);
    return saturatef(1.0f - cc);
}

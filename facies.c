#include "facies.h"
#include "sors.h"
#include "sdf.h"
#include <string.h>

void facies_default(FaciesParametra* p) {
    if (!p)
        return;
    memset(p, 0, sizeof(*p));
    p->semen = 1;
    p->tempus = 0.0f;
    p->forma_faciei = FORMA_OVALIS;
    p->gens = GENS_HUMANA;
    p->archetypum = ARCH_LAR;

    p->aetas = 0.4f;
    p->color_cutis = 0.35f;
    p->calor_cutis = 0.5f;
    p->masculinitas = 0.6f;

    p->latitudo_faciei = 0.5f;
    p->altitudo_faciei = 0.5f;
    p->latitudo_malae = 0.5f;
    p->quadratura_malae = 0.4f;
    p->plenitudo_genae = 0.5f;
    p->rima_menti = 0.1f;

    p->magnitudo_oculi = 0.5f;
    p->distantia_oculorum = 0.5f;
    p->inclinatio_oculi = 0.5f;
    p->pondus_palpebrae = 0.3f;
    p->plica_epicanthica = 0.0f;
    p->magnitudo_iridis = 0.5f;
    p->color_iridis_h = 0.1f;  /* brunneus */
    p->color_iridis_s = 0.55f;
    p->venae_sclerae = 0.1f;

    p->altitudo_supercilii = 0.5f;
    p->arcus_supercilii = 0.5f;
    p->crassitudo_supercilii = 0.4f;
    p->longitudo_supercilii = 0.55f;
    p->intervallum_supercilii = 0.5f;

    p->latitudo_nasi = 0.5f;
    p->longitudo_nasi = 0.55f;
    p->altitudo_pontis = 0.4f;
    p->latitudo_pontis = 0.4f;
    p->forma_apicis = 0.5f;
    p->curvatura_nasi = 0.5f;
    p->dilatatio_narium = 0.35f;

    p->latitudo_oris = 0.5f;
    p->altitudo_oris = 0.5f;
    p->crassitudo_labii_sup = 0.45f;
    p->crassitudo_labii_inf = 0.55f;
    p->arcus_cupidinis = 0.5f;
    p->profunditas_philtri = 0.3f;
    p->angulus_anguli_oris = 0.0f;

    p->magnitudo_auris = 0.5f;
    p->protrusio_auris = 0.3f;
    p->magnitudo_lobi = 0.4f;
    p->acies_auris = 0.0f;

    p->modus_comae = COMA_MEDIA_UNDATA;
    p->volumen_comae = 0.55f;
    p->longitudo_comae = 0.5f;
    p->altitudo_frontis = 0.5f;
    p->recessus_frontis = 0.2f;
    p->color_comae_h = 0.08f;
    p->color_comae_s = 0.6f;
    p->color_comae_v = 0.25f;
    p->canitudo = 0.0f;

    p->modus_barbae = BARBA_NULLA;
    p->densitas_barbae = 0.5f;
    p->modus_mustaciorum = MUSTACIA_NULLA;

    p->modus_vestis = VESTIS_TUNICA;
    p->color_vestis_h = 0.0f;
    p->color_vestis_s = 0.55f;
    p->color_vestis_v = 0.35f;
    p->color_ornamenti_h = 0.1f;

    p->modus_ornamenti = ORNAMENTUM_NULLUM;
    p->color_capitis_h = 0.1f;
    p->heterochromia = 0.0f;
    p->color_iridis2_h = 0.6f;
    p->color_iridis2_s = 0.55f;
    p->oculi_asymmetria = 0.0f;

    p->densitas_lentiginorum = 0.1f;
    p->profunditas_rugarum = 0.2f;
    p->numerus_cicatricum = 0.0f;
    p->visibilitas_pororum = 0.4f;

    p->magnitudo_dentis = 0.0f;
    p->modus_dentium = DENTES_NULLI;
    p->magnitudo_cornu = 0.0f;
    p->bioluminescentia = 0.0f;
}

void facies_genera(FaciesParametra* p, uint64_t semen) {
    facies_default(p);
    Sors s = sors_inita(semen);
    p->semen = semen;

    {
        /* Formae faciēī pōnderātae — ovāleās et rotundās plūs commūnēs, quadrātās rāriōrēs */
        float pond_fm[FORMA_NUMERUS];
        pond_fm[FORMA_OVALIS]       = 0.38f;
        pond_fm[FORMA_ROTUNDA]      = 0.18f;
        pond_fm[FORMA_OBLONGA]      = 0.12f;
        pond_fm[FORMA_CORDIS]       = 0.09f;
        pond_fm[FORMA_TRIANGULARIS] = 0.07f;
        pond_fm[FORMA_RHOMBI]       = 0.07f;
        pond_fm[FORMA_QUADRATA]     = 0.05f;
        pond_fm[FORMA_RECTA]        = 0.04f;
        p->forma_faciei = (FormaFaciei) sors_ponderatus(&s, pond_fm, FORMA_NUMERUS);
    }
    p->aetas        = saturatef(sors_gauss(&s, 0.45f, 0.2f));
    /* Cutis: distributio favet extrēma — U-shaped ut pallidissimī et obscūrissimī plūs appāreant */
    {
        float r = sors_f32(&s);
        float u = (r - 0.5f) * 2.0f;      /* [-1..1] */
        float u3 = u * u * u;              /* cubica favet extrēma */
        p->color_cutis = saturatef(0.5f + 0.5f * u3);
    }
    p->calor_cutis  = sors_gauss(&s, 0.5f, 0.18f);
    p->masculinitas = sors_f32(&s);

    /* Sigmae latiores ut variētās intra archetypum conspicua sit */
    p->latitudo_faciei = saturatef(sors_gauss(&s, 0.5f, 0.20f));
    p->altitudo_faciei = saturatef(sors_gauss(&s, 0.5f, 0.20f));
    p->latitudo_malae  = saturatef(sors_gauss(&s, 0.5f, 0.22f));
    p->quadratura_malae = saturatef(sors_gauss(&s, 0.4f, 0.26f));
    p->plenitudo_genae = saturatef(sors_gauss(&s, 0.5f, 0.24f));
    p->rima_menti      = saturatef(sors_f32(&s) * 0.3f);

    p->magnitudo_oculi       = saturatef(sors_gauss(&s, 0.5f, 0.14f));
    p->distantia_oculorum    = saturatef(sors_gauss(&s, 0.5f, 0.13f));
    p->inclinatio_oculi      = saturatef(sors_gauss(&s, 0.5f, 0.15f));
    p->pondus_palpebrae      = saturatef(sors_gauss(&s, 0.3f, 0.22f));
    p->plica_epicanthica     = saturatef(sors_f32(&s) * 0.6f);
    p->magnitudo_iridis      = saturatef(sors_gauss(&s, 0.5f, 0.1f));
    {
        /* Color iridis: palette lata.
         * 0=brunneus 1=caeruleus 2=viridis 3=canus 4=hazel 5=ambar 6=violaceus
         * 7=turquoise 8=aureus 9=sanguineus 10=albus (spectralis) */
        float hues[] = {
            0.08f, 0.58f, 0.33f, 0.08f, 0.10f,
            0.10f, 0.78f, 0.47f, 0.14f, 0.99f, 0.55f
        };
        float pond[] = {
            0.20f, 0.12f, 0.08f, 0.08f, 0.10f,   /* naturales: 58% */
            0.08f,                                 /* ambar */
            0.09f, 0.08f,                          /* violaceus, turquoise */
            0.06f, 0.06f, 0.05f                    /* aureus, sanguineus, albus — 34% wild */
        };
        int i = sors_ponderatus(&s, pond, 11);
        p->color_iridis_h = hues[i] + sors_spatium(&s, -0.02f, 0.02f);
        if (i == 3)
            p->color_iridis_s = sors_spatium(&s, 0.05f, 0.20f);  /* canus desaturatus */
        else if (i == 10)
            p->color_iridis_s = sors_spatium(&s, 0.00f, 0.08f);  /* albus spectralis */
        else if (i >= 5)
            p->color_iridis_s = saturatef(sors_gauss(&s, 0.85f, 0.10f));  /* exotici saturati */
        else
            p->color_iridis_s = saturatef(sors_gauss(&s, 0.60f, 0.20f));
    }
    p->venae_sclerae         = saturatef(sors_f32(&s) * 0.3f);

    p->altitudo_supercilii   = saturatef(sors_gauss(&s, 0.5f, 0.12f));
    p->arcus_supercilii      = saturatef(sors_gauss(&s, 0.5f, 0.16f));
    p->crassitudo_supercilii = saturatef(sors_gauss(&s, 0.4f, 0.2f));
    p->longitudo_supercilii  = saturatef(sors_gauss(&s, 0.55f, 0.13f));
    p->intervallum_supercilii = saturatef(sors_gauss(&s, 0.5f, 0.12f));

    p->latitudo_nasi   = saturatef(sors_gauss(&s, 0.5f, 0.12f));
    p->longitudo_nasi  = saturatef(sors_gauss(&s, 0.55f, 0.15f));
    p->altitudo_pontis = saturatef(sors_gauss(&s, 0.4f, 0.2f));
    p->latitudo_pontis = saturatef(sors_gauss(&s, 0.4f, 0.2f));
    p->forma_apicis    = sors_f32(&s);
    p->curvatura_nasi  = saturatef(sors_gauss(&s, 0.5f, 0.15f));
    p->dilatatio_narium = saturatef(sors_gauss(&s, 0.35f, 0.18f));

    p->latitudo_oris         = saturatef(sors_gauss(&s, 0.5f, 0.12f));
    p->altitudo_oris         = saturatef(sors_gauss(&s, 0.5f, 0.1f));
    p->crassitudo_labii_sup  = saturatef(sors_gauss(&s, 0.45f, 0.18f));
    p->crassitudo_labii_inf  = saturatef(sors_gauss(&s, 0.55f, 0.18f));
    p->arcus_cupidinis       = saturatef(sors_gauss(&s, 0.55f, 0.18f));
    p->profunditas_philtri   = saturatef(sors_gauss(&s, 0.3f, 0.18f));
    p->angulus_anguli_oris   = sors_gauss(&s, 0.0f, 0.25f);  /* - / + */

    p->magnitudo_auris = saturatef(sors_gauss(&s, 0.5f, 0.12f));
    p->protrusio_auris = saturatef(sors_gauss(&s, 0.3f, 0.18f));
    p->magnitudo_lobi  = saturatef(sors_gauss(&s, 0.4f, 0.18f));
    /* acies_auris: humani fere rotundi, rare acuti; variatio intra gentem */
    p->acies_auris     = (sors_f32(&s) < 0.12f) ? saturatef(sors_gauss(&s, 0.6f, 0.25f)) : 0.0f;

    p->modus_comae       = (ModusComae)(sors_proximus(&s) % COMA_NUMERUS);
    p->volumen_comae     = saturatef(sors_gauss(&s, 0.55f, 0.2f));
    p->longitudo_comae   = saturatef(sors_gauss(&s, 0.4f, 0.3f));
    p->altitudo_frontis  = saturatef(sors_gauss(&s, 0.5f, 0.13f));
    p->recessus_frontis  = saturatef(sors_f32(&s) * 0.4f);
    {
        /* Plerumque naturalis: brunneus, niger, rufus, flavus. ~10% exoticus (phantasticus).
         * hues: 0=brunneus 1=niger 2=obscurus 3=deep-dark 4=flavus 5=rufus
         *       6=argenteus 7=viridis 8=caeruleus 9=purpureus 10=roseus */
        float hues_comae[] = { 0.08f, 0.06f, 0.02f, 0.0f, 0.13f, 0.03f,
            0.0f,  0.30f, 0.58f, 0.78f, 0.92f };
        float pond[] =       { 0.30f, 0.22f, 0.12f, 0.08f, 0.13f, 0.07f,
            0.020f, 0.015f, 0.020f, 0.020f, 0.015f };
        int i = sors_ponderatus(&s, pond, 11);
        p->color_comae_h = hues_comae[i] + sors_spatium(&s, -0.01f, 0.01f);
        p->color_comae_s = saturatef(sors_gauss(&s, 0.55f, 0.2f));
        p->color_comae_v = saturatef(sors_gauss(&s, 0.3f, 0.2f));
        if (i == 4)
            p->color_comae_v = saturatef(sors_gauss(&s, 0.65f, 0.1f));  /* flavus clarior */
        else if (i == 5)
            p->color_comae_s = saturatef(sors_gauss(&s, 0.75f, 0.1f));  /* rufus saturatior */
        else if (i == 6) {
            p->color_comae_s = sors_spatium(&s, 0.0f, 0.08f);            /* argenteus */
            p->color_comae_v = sors_spatium(&s, 0.70f, 0.90f);
        } else if (i >= 7) {
            p->color_comae_s = saturatef(sors_gauss(&s, 0.80f, 0.12f));  /* exoticus vivus */
            p->color_comae_v = saturatef(sors_gauss(&s, 0.45f, 0.12f));
        }
    }

    p->canitudo = p->aetas > 0.6f ? saturatef((p->aetas - 0.6f) * 2.5f * sors_f32(&s)) : 0.0f;

    /* Barba: tantum si masculinitas alta; ponderatio favet variantes visibiliter distinctas */
    if (p->masculinitas > 0.55f && sors_f32(&s) < 0.6f) {
        float pond_b[BARBA_NUMERUS];
        pond_b[BARBA_NULLA]    = 0.00f;
        pond_b[BARBA_STIRPS]   = 0.22f;
        pond_b[BARBA_MAXILLAE] = 0.22f;
        pond_b[BARBA_PLENA]    = 0.20f;
        pond_b[BARBA_PROLIXA]  = 0.18f;
        pond_b[BARBA_BIFIDA]   = 0.18f;
        p->modus_barbae = (ModusBarbae) sors_ponderatus(&s, pond_b, BARBA_NUMERUS);
        p->densitas_barbae = saturatef(sors_gauss(&s, 0.65f, 0.22f));
        p->modus_mustaciorum = (ModusMustaciorum)(sors_proximus(&s) % MUSTACIA_NUMERUS);
    } else {
        p->modus_barbae = BARBA_NULLA;
        p->modus_mustaciorum = MUSTACIA_NULLA;
    }

    /* Vestimenta — eligit modum ponderate et colorem */
    {
        float pond_v[VESTIS_NUMERUS];
        pond_v[VESTIS_NULLA]   = 0.06f;
        pond_v[VESTIS_TUNICA]  = 0.34f;
        pond_v[VESTIS_TOGA]    = 0.16f;
        pond_v[VESTIS_STOLA]   = 0.14f;
        pond_v[VESTIS_LORICA]  = 0.15f;
        pond_v[VESTIS_PALLIUM] = 0.15f;
        p->modus_vestis = (ModusVestis) sors_ponderatus(&s, pond_v, VESTIS_NUMERUS);

        p->color_vestis_h = sors_f32(&s);
        p->color_vestis_s = saturatef(sors_gauss(&s, 0.60f, 0.18f));
        p->color_vestis_v = saturatef(sors_gauss(&s, 0.35f, 0.12f));
        p->color_ornamenti_h = sors_f32(&s);

        /* Modulat colorem secundum modum vestis */
        switch (p->modus_vestis) {
        case VESTIS_LORICA:
            /* metallicus: aeneus aut ferreus */
            p->color_vestis_h = sors_f32(&s) < 0.55f ? 0.07f : 0.60f;
            p->color_vestis_s = sors_spatium(&s, 0.18f, 0.35f);
            p->color_vestis_v = sors_spatium(&s, 0.28f, 0.42f);
            break;
        case VESTIS_TOGA:
            /* alba paene: creamy off-white cum clavo purpureo */
            p->color_vestis_h = 0.10f;
            p->color_vestis_s = 0.08f;
            p->color_vestis_v = sors_spatium(&s, 0.72f, 0.82f);
            p->color_ornamenti_h = 0.93f;  /* purpura clavi */
            break;
        case VESTIS_STOLA:
            /* pastellus muliebris */
            p->color_vestis_s = sors_spatium(&s, 0.30f, 0.55f);
            p->color_vestis_v = sors_spatium(&s, 0.55f, 0.78f);
            break;
        case VESTIS_TUNICA:
        case VESTIS_PALLIUM:
            /* colores pleni saturati */
            p->color_vestis_s = sors_spatium(&s, 0.55f, 0.85f);
            p->color_vestis_v = sors_spatium(&s, 0.22f, 0.48f);
            break;
        default:
            break;
        }
    }

    /* Ornamenta capitis — eligit modum ponderate */
    {
        float pond_o[ORNAMENTUM_NUMERUS];
        pond_o[ORNAMENTUM_NULLUM]        = 0.65f;
        pond_o[ORNAMENTUM_CORONA_LAUREA] = 0.08f;
        pond_o[ORNAMENTUM_GALEA]         = 0.06f;
        pond_o[ORNAMENTUM_VITTA]         = 0.06f;
        pond_o[ORNAMENTUM_PILEUS]        = 0.05f;
        pond_o[ORNAMENTUM_DIADEMA]       = 0.05f;
        pond_o[ORNAMENTUM_FEX]           = 0.05f;
        p->modus_ornamenti = (ModusOrnamenti) sors_ponderatus(&s, pond_o, ORNAMENTUM_NUMERUS);
        p->color_capitis_h = sors_f32(&s);
    }

    /* Heterochromia: 10% probabilitas; secundus oculus ex palette lata */
    if (sors_f32(&s) < 0.10f) {
        p->heterochromia = 1.0f;
        float hues[] = {
            0.08f, 0.58f, 0.33f, 0.10f, 0.78f,
            0.47f, 0.14f, 0.99f
        };
        int i = sors_proximus(&s) % 8;
        p->color_iridis2_h = hues[i];
        p->color_iridis2_s = saturatef(sors_gauss(&s, 0.70f, 0.18f));
    } else {
        p->heterochromia = 0.0f;
        p->color_iridis2_h = p->color_iridis_h;
        p->color_iridis2_s = p->color_iridis_s;
    }

    /* Asymmetria oculorum — nunc per latus signata (negativa=sin maior) */
    p->oculi_asymmetria = sors_gauss(&s, 0.0f, 0.28f);
    if (p->oculi_asymmetria > 1.0f)
        p->oculi_asymmetria = 1.0f;
    if (p->oculi_asymmetria < -1.0f)
        p->oculi_asymmetria = -1.0f;

    /* Dentes: rarissimi ut nulli plerumque; si apparent, varia forma */
    {
        float pond_d[DENTES_NUMERUS];
        pond_d[DENTES_NULLI]         = 0.55f;
        pond_d[DENTES_CANINI_INFERI] = 0.06f;    /* uptooth — ex mandibula */
        pond_d[DENTES_CANINI_SUPERI] = 0.06f;    /* fanga — ex maxilla */
        pond_d[DENTES_SINGULARIS]    = 0.10f;
        pond_d[DENTES_RUPTI]         = 0.09f;
        pond_d[DENTES_ORDO]          = 0.14f;
        p->modus_dentium = (ModusDentium) sors_ponderatus(&s, pond_d, DENTES_NUMERUS);
        p->magnitudo_dentis = (p->modus_dentium == DENTES_NULLI)
            ? 0.0f : saturatef(sors_gauss(&s, 0.55f, 0.18f));
        /* ORDO requirit os apertum aut angulum elevatum ut dentes videantur */
        if (p->modus_dentium == DENTES_ORDO)
            p->altitudo_oris = fmaxf(p->altitudo_oris, 0.68f);
    }

    p->densitas_lentiginorum = saturatef(sors_f32(&s) * 0.7f);
    p->profunditas_rugarum   = saturatef(p->aetas * 0.7f + sors_spatium(&s, -0.1f, 0.1f));
    p->numerus_cicatricum    = (sors_f32(&s) < 0.08f) ? sors_f32(&s) * 0.4f : 0.0f;
    p->visibilitas_pororum   = saturatef(sors_gauss(&s, 0.4f, 0.15f));
}

/* --- Forma SDF silhouette ---
 *
 * Facies est ellipsis distorta ad formam specificam. Distortio per "warp":
 * pinch superior/inferior, inclinatio malarum, etc. */
float sdf_contura_faciei(vec2 p, const FaciesParametra* par, const ZonaeFaciei* z) {
    vec2 q = v2_sub(p, z->centrum_faciei);

    /* Distortio per formam */
    float pinch_top = 0.0f;    /* [-1..1] — negatif = stringit apicem */
    float pinch_bot = 0.0f;
    float wide_mid  = 0.0f;    /* 0..1 - latius in medio */
    switch (par->forma_faciei) {
    case FORMA_OVALIS:       pinch_top = -0.15f;
        pinch_bot = -0.15f;
        break;
    case FORMA_ROTUNDA:      pinch_top = -0.05f;
        pinch_bot = -0.05f;
        break;
    case FORMA_QUADRATA:     pinch_top =  0.05f;
        pinch_bot = -0.05f;
        wide_mid = 0.10f;
        break;
    case FORMA_CORDIS:       pinch_top =  0.20f;
        pinch_bot = -0.30f;
        break;
    case FORMA_RHOMBI:       pinch_top = -0.20f;
        pinch_bot = -0.20f;
        wide_mid = 0.15f;
        break;
    case FORMA_OBLONGA:      pinch_top = -0.10f;
        pinch_bot = -0.10f;
        break;
    case FORMA_TRIANGULARIS: pinch_top = -0.25f;
        pinch_bot =  0.15f;
        break;
    case FORMA_RECTA:        pinch_top =  0.0f;
        pinch_bot =  0.0f;
        break;
    default: break;
    }

    float rx = z->lat_faciei;
    float ry = z->alt_faciei;

    /* Modulatio rx secundum y: pinchet apices et extendit medium */
    float y_norm = q.y / (ry + 1e-10f);
    float mod;
    if (y_norm < 0.0f) {
        mod = 1.0f + pinch_top * (-y_norm);
    } else {
        mod = 1.0f + pinch_bot * y_norm;
    }
    mod += wide_mid * (1.0f - y_norm * y_norm);

    float rx_eff = rx * mod;

    /* Forma quadrata — capsa rotundata pro FORMA_QUADRATA/RECTA */
    if (par->forma_faciei == FORMA_QUADRATA || par->forma_faciei == FORMA_RECTA) {
        float corner = 0.42f * fminf(rx, ry);
        return sdf_capsa_rotunda(q, v2(rx_eff - corner, ry - corner), corner);
    }

    /* Defaulta: ellipsis scalata */
    return sdf_ellipsis(q, rx_eff, ry);
}

ZonaeFaciei zonae_computa(const FaciesParametra* p, int w, int h) {
    ZonaeFaciei z;
    memset(&z, 0, sizeof(z));

    z.centrum_faciei.x = w * 0.5f;
    z.centrum_faciei.y = h * 0.56f;   /* centrum faciei sub medio imaginis */

    /* Magnitudo faciei — relictis spatiis pro coma superius et umeris inferius */
    float baseH = h * 0.34f;
    float baseW = w * 0.28f;

    float gens_mult_w = 1.0f, gens_mult_h = 1.0f;
    switch (p->gens) {
    case GENS_NYMPHARUM:     gens_mult_h = 1.08f;
        gens_mult_w = 0.92f;
        break;
    case GENS_PYGMAEORUM:  gens_mult_h = 0.85f;
        gens_mult_w = 1.05f;
        break;
    case GENS_GIGANTUM:     gens_mult_h = 0.98f;
        gens_mult_w = 1.08f;
        break;
    case GENS_PENATIUM: gens_mult_h = 0.90f;
        gens_mult_w = 1.00f;
        break;
    case GENS_SATYRORUM:    gens_mult_h = 0.80f;
        gens_mult_w = 0.95f;
        break;
    default: break;
    }

    z.lat_faciei = baseW * (0.9f + 0.3f * p->latitudo_faciei) * gens_mult_w;
    z.alt_faciei = baseH * (0.9f + 0.25f * p->altitudo_faciei) * gens_mult_h;

    /* Animatio respiratio: scala leviter oscillat */
    float resp = sinf(PORTRAIT_TAU * p->tempus) * 0.003f + 1.0f;
    z.lat_faciei *= resp;
    z.alt_faciei *= resp;

    /* Oculi */
    float dist_oculi = z.lat_faciei * (0.32f + 0.15f * p->distantia_oculorum);
    z.oculi.y         = z.centrum_faciei.y - z.alt_faciei * 0.12f;
    z.oculi.sin       = v2(z.centrum_faciei.x - dist_oculi, z.oculi.y);
    z.oculi.dex       = v2(z.centrum_faciei.x + dist_oculi, z.oculi.y);
    z.oculi.distantia = dist_oculi * 2.0f;
    z.oculi.r_oculi_x = z.lat_faciei * (0.12f + 0.08f * p->magnitudo_oculi);
    z.oculi.r_oculi_y = z.oculi.r_oculi_x * (0.50f + 0.10f * p->magnitudo_oculi);
    z.oculi.r_iridis  = z.oculi.r_oculi_x * (0.55f + 0.25f * p->magnitudo_iridis);
    z.oculi.inclinatio = (p->inclinatio_oculi - 0.5f) * 0.5f;

    /* Nasus */
    z.nasus.centrum = v2(z.centrum_faciei.x, z.centrum_faciei.y + z.alt_faciei * 0.10f);
    z.nasus.lat     = z.lat_faciei * (0.14f + 0.12f * p->latitudo_nasi);
    z.nasus.alt     = z.alt_faciei * (0.22f + 0.14f * p->longitudo_nasi);
    z.nasus.pontis_lat = z.nasus.lat * (0.2f + 0.5f * p->latitudo_pontis);

    /* Os */
    z.os.centrum = v2(z.centrum_faciei.x, z.centrum_faciei.y + z.alt_faciei * 0.40f);
    z.os.lat     = z.lat_faciei * (0.24f + 0.16f * p->latitudo_oris);
    z.os.alt     = z.alt_faciei * (0.07f + 0.07f * p->altitudo_oris);
    z.os.labium_sup_t = p->crassitudo_labii_sup;
    z.os.labium_inf_t = p->crassitudo_labii_inf;

    /* Aures */
    z.aures.y   = z.oculi.y + z.alt_faciei * 0.05f;
    z.aures.lat = z.lat_faciei * (0.09f + 0.05f * p->magnitudo_auris);
    z.aures.alt = z.alt_faciei * (0.17f + 0.08f * p->magnitudo_auris);
    /* Aures ad marginem lateralem faciei — ancorae interior, vertex protrudit modice */
    float ear_x = z.lat_faciei * (0.82f + 0.12f * p->protrusio_auris);
    z.aures.sin = v2(z.centrum_faciei.x - ear_x, z.aures.y);
    z.aures.dex = v2(z.centrum_faciei.x + ear_x, z.aures.y);
    /* Gens-specific bias miscetur cum parametro individuali ut variantia intra gentem servetur */
    {
        float gens_min = 0.0f, gens_max = 0.0f;
        switch (p->gens) {
        case GENS_NYMPHARUM: gens_min = 0.65f;
            gens_max = 1.00f;
            break;
        case GENS_PENATIUM:  gens_min = 0.35f;
            gens_max = 0.70f;
            break;
        case GENS_SATYRORUM: gens_min = 0.30f;
            gens_max = 0.65f;
            break;
        case GENS_FURIARUM:  gens_min = 0.25f;
            gens_max = 0.60f;
            break;
        default: break;
        }
        if (gens_max > 0.0f)
            z.aures.acies = gens_min + (gens_max - gens_min) * (0.5f + 0.5f * p->acies_auris);
        else
            z.aures.acies = p->acies_auris;
    }

    /* Mentum */
    z.mentum.centrum = v2(z.centrum_faciei.x, z.centrum_faciei.y + z.alt_faciei * 0.88f);
    z.mentum.r       = z.lat_faciei * 0.22f;

    /* Frons */
    z.frons.y_frons  = z.oculi.y - z.alt_faciei * 0.28f;
    z.frons.y_culmen = z.centrum_faciei.y - z.alt_faciei * (0.95f - 0.1f * p->altitudo_frontis);
    z.frons.recessus = p->recessus_frontis;

    z.cervix.y_sup = z.centrum_faciei.y + z.alt_faciei * 0.95f;
    z.cervix.y_inf = (float)h;

    /* Supercilia */
    float sup_y = z.oculi.y - z.oculi.r_oculi_y - z.alt_faciei * (0.04f + 0.06f * p->altitudo_supercilii);
    z.supercilia_sin.centrum = v2(z.oculi.sin.x, sup_y);
    z.supercilia_dex.centrum = v2(z.oculi.dex.x, sup_y);
    z.supercilia_sin.lat = z.oculi.r_oculi_x * (1.0f + 0.5f * p->longitudo_supercilii);
    z.supercilia_dex.lat = z.supercilia_sin.lat;
    z.supercilia_sin.alt = 1.0f + 2.0f * p->crassitudo_supercilii;
    z.supercilia_dex.alt = z.supercilia_sin.alt;

    return z;
}

/* --- Clavium mutabilium per nomen --- */
typedef struct {
    const char* nomen;
    size_t      offset;
} Clavis;

static const Clavis CLAVES[] = {
    { "aetas",                  offsetof(FaciesParametra, aetas) },
    { "color_cutis",            offsetof(FaciesParametra, color_cutis) },
    { "calor_cutis",            offsetof(FaciesParametra, calor_cutis) },
    { "masculinitas",           offsetof(FaciesParametra, masculinitas) },
    { "latitudo_faciei",        offsetof(FaciesParametra, latitudo_faciei) },
    { "altitudo_faciei",        offsetof(FaciesParametra, altitudo_faciei) },
    { "latitudo_malae",         offsetof(FaciesParametra, latitudo_malae) },
    { "quadratura_malae",       offsetof(FaciesParametra, quadratura_malae) },
    { "plenitudo_genae",        offsetof(FaciesParametra, plenitudo_genae) },
    { "rima_menti",             offsetof(FaciesParametra, rima_menti) },
    { "magnitudo_oculi",        offsetof(FaciesParametra, magnitudo_oculi) },
    { "distantia_oculorum",     offsetof(FaciesParametra, distantia_oculorum) },
    { "inclinatio_oculi",       offsetof(FaciesParametra, inclinatio_oculi) },
    { "pondus_palpebrae",       offsetof(FaciesParametra, pondus_palpebrae) },
    { "plica_epicanthica",      offsetof(FaciesParametra, plica_epicanthica) },
    { "magnitudo_iridis",       offsetof(FaciesParametra, magnitudo_iridis) },
    { "color_iridis_h",         offsetof(FaciesParametra, color_iridis_h) },
    { "color_iridis_s",         offsetof(FaciesParametra, color_iridis_s) },
    { "venae_sclerae",          offsetof(FaciesParametra, venae_sclerae) },
    { "altitudo_supercilii",    offsetof(FaciesParametra, altitudo_supercilii) },
    { "arcus_supercilii",       offsetof(FaciesParametra, arcus_supercilii) },
    { "crassitudo_supercilii",  offsetof(FaciesParametra, crassitudo_supercilii) },
    { "longitudo_supercilii",   offsetof(FaciesParametra, longitudo_supercilii) },
    { "intervallum_supercilii", offsetof(FaciesParametra, intervallum_supercilii) },
    { "latitudo_nasi",          offsetof(FaciesParametra, latitudo_nasi) },
    { "longitudo_nasi",         offsetof(FaciesParametra, longitudo_nasi) },
    { "altitudo_pontis",        offsetof(FaciesParametra, altitudo_pontis) },
    { "latitudo_pontis",        offsetof(FaciesParametra, latitudo_pontis) },
    { "forma_apicis",           offsetof(FaciesParametra, forma_apicis) },
    { "curvatura_nasi",         offsetof(FaciesParametra, curvatura_nasi) },
    { "dilatatio_narium",       offsetof(FaciesParametra, dilatatio_narium) },
    { "latitudo_oris",          offsetof(FaciesParametra, latitudo_oris) },
    { "altitudo_oris",          offsetof(FaciesParametra, altitudo_oris) },
    { "crassitudo_labii_sup",   offsetof(FaciesParametra, crassitudo_labii_sup) },
    { "crassitudo_labii_inf",   offsetof(FaciesParametra, crassitudo_labii_inf) },
    { "arcus_cupidinis",        offsetof(FaciesParametra, arcus_cupidinis) },
    { "profunditas_philtri",    offsetof(FaciesParametra, profunditas_philtri) },
    { "angulus_anguli_oris",    offsetof(FaciesParametra, angulus_anguli_oris) },
    { "magnitudo_auris",        offsetof(FaciesParametra, magnitudo_auris) },
    { "protrusio_auris",        offsetof(FaciesParametra, protrusio_auris) },
    { "magnitudo_lobi",         offsetof(FaciesParametra, magnitudo_lobi) },
    { "acies_auris",            offsetof(FaciesParametra, acies_auris) },
    { "volumen_comae",          offsetof(FaciesParametra, volumen_comae) },
    { "longitudo_comae",        offsetof(FaciesParametra, longitudo_comae) },
    { "altitudo_frontis",       offsetof(FaciesParametra, altitudo_frontis) },
    { "recessus_frontis",       offsetof(FaciesParametra, recessus_frontis) },
    { "color_comae_h",          offsetof(FaciesParametra, color_comae_h) },
    { "color_comae_s",          offsetof(FaciesParametra, color_comae_s) },
    { "color_comae_v",          offsetof(FaciesParametra, color_comae_v) },
    { "canitudo",               offsetof(FaciesParametra, canitudo) },
    { "densitas_barbae",        offsetof(FaciesParametra, densitas_barbae) },
    { "densitas_lentiginorum",  offsetof(FaciesParametra, densitas_lentiginorum) },
    { "profunditas_rugarum",    offsetof(FaciesParametra, profunditas_rugarum) },
    { "numerus_cicatricum",     offsetof(FaciesParametra, numerus_cicatricum) },
    { "visibilitas_pororum",    offsetof(FaciesParametra, visibilitas_pororum) },
    { "magnitudo_dentis",       offsetof(FaciesParametra, magnitudo_dentis) },
    { "magnitudo_cornu",        offsetof(FaciesParametra, magnitudo_cornu) },
    { "bioluminescentia",       offsetof(FaciesParametra, bioluminescentia) },
};

int facies_pone_clavem(FaciesParametra* p, const char* clavis, float valor) {
    if (!p || !clavis)
        return 0;
    size_t n = sizeof(CLAVES) / sizeof(CLAVES[0]);
    for (size_t i = 0; i < n; i++) {
        if (strcmp(clavis, CLAVES[i].nomen) == 0) {
            float* f = (float*)((char*)p + CLAVES[i].offset);
            *f = valor;
            return 1;
        }
    }
    return 0;
}

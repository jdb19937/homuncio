#include "archetypum.h"

/* Helper: miscet parametrum inter valorem generatum et valorem prior, cum
 *  `vis` indicante quanti archetypus valorem dirigat. */
static inline void dir(float* p, float prior, float vis) {
    *p = saturatef((*p) * (1.0f - vis) + prior * vis);
}

void archetypum_applica(FaciesParametra* p, Archetypum arch) {
    if (!p)
        return;
    p->archetypum = arch;
    switch (arch) {
    case ARCH_LAR:
            /* Spiritus domus custos — iuvenilis fortis, cutis calida */
        dir(&p->latitudo_malae, 0.65f, 0.25f);
        dir(&p->quadratura_malae, 0.65f, 0.25f);
        dir(&p->altitudo_frontis, 0.7f, 0.3f);
        dir(&p->calor_cutis, 0.7f, 0.3f);
        dir(&p->color_iridis_s, 0.75f, 0.35f);
        dir(&p->inclinatio_oculi, 0.55f, 0.3f);
        dir(&p->masculinitas, 0.8f, 0.3f);
        if (
            p->modus_ornamenti == ORNAMENTUM_NULLUM ||
            p->modus_ornamenti == ORNAMENTUM_CORONA_LAUREA
        )
            p->modus_ornamenti = ORNAMENTUM_CORONA_LAUREA;
        break;
    case ARCH_MANES:
            /* Spiritus maiorum divini — aetatis plenae, rugae profundae, canitudo */
        dir(&p->aetas, 0.9f, 0.8f);
        p->canitudo = saturatef(p->canitudo + 0.7f);
        dir(&p->profunditas_rugarum, 0.85f, 0.7f);
        dir(&p->pondus_palpebrae, 0.7f, 0.5f);
        dir(&p->color_comae_v, 0.85f, 0.5f);
        dir(&p->color_comae_s, 0.10f, 0.8f);
        break;
    case ARCH_LEMUR:
            /* Spiritus malevolus mortuus — oculi gravati, toni obscuri, labia tenuia */
        dir(&p->inclinatio_oculi, 0.75f, 0.5f);
        dir(&p->pondus_palpebrae, 0.55f, 0.4f);
        dir(&p->arcus_supercilii, 0.75f, 0.5f);
        dir(&p->altitudo_supercilii, 0.30f, 0.5f);
        dir(&p->color_cutis, 0.65f, 0.3f);
        dir(&p->calor_cutis, 0.35f, 0.5f);
        dir(&p->crassitudo_labii_sup, 0.30f, 0.4f);
        dir(&p->crassitudo_labii_inf, 0.30f, 0.4f);
        /* Mortuus inquietus: dentes rupti, si apparent */
        if (p->modus_dentium != DENTES_NULLI)
            p->modus_dentium = DENTES_RUPTI;
        break;
    case ARCH_IANUS:
            /* Deus bifrons liminum — supercilia expressa, callidus */
        dir(&p->arcus_supercilii, 0.75f, 0.5f);
        dir(&p->magnitudo_oculi, 0.45f, 0.3f);
        dir(&p->angulus_anguli_oris, 0.3f, 0.3f);  /* subrisus */
            /* asymmetria fit in generatione iniiali */
        break;
    case ARCH_AUGUR:
            /* Sacerdos interpres ominum — nasus prominens, cutis pallida */
        dir(&p->longitudo_nasi, 0.7f, 0.4f);
        dir(&p->altitudo_frontis, 0.75f, 0.4f);
        dir(&p->color_cutis, 0.2f, 0.3f);
        dir(&p->profunditas_rugarum, 0.4f, 0.2f);
        dir(&p->arcus_supercilii, 0.6f, 0.3f);
        if (p->modus_vestis != VESTIS_NULLA)
            p->modus_vestis = VESTIS_TOGA;
        p->modus_ornamenti = ORNAMENTUM_VITTA;
        break;
    case ARCH_QUIRINUS:
            /* Romulus divinus, deus militaris — cicatrices, cutis tacta intemperie */
        p->numerus_cicatricum = fmaxf(p->numerus_cicatricum, 0.3f);
        dir(&p->quadratura_malae, 0.75f, 0.3f);
        dir(&p->masculinitas, 0.9f, 0.5f);
        dir(&p->calor_cutis, 0.75f, 0.3f);
        dir(&p->pondus_palpebrae, 0.45f, 0.3f);
        if (p->modus_vestis != VESTIS_NULLA)
            p->modus_vestis = VESTIS_LORICA;
            /* Galea militaris fere semper */
        if (p->modus_ornamenti != ORNAMENTUM_CORONA_LAUREA)
            p->modus_ornamenti = ORNAMENTUM_GALEA;
        break;
    case ARCH_CARNA:
            /* Dea viscerum et valetudinis — lineamenta mollia, cutis calida */
        dir(&p->plenitudo_genae, 0.6f, 0.3f);
        dir(&p->crassitudo_labii_sup, 0.6f, 0.3f);
        dir(&p->crassitudo_labii_inf, 0.65f, 0.3f);
        dir(&p->altitudo_supercilii, 0.6f, 0.3f);
        dir(&p->arcus_supercilii, 0.6f, 0.3f);
        dir(&p->angulus_anguli_oris, 0.55f, 0.3f);
        if (p->modus_vestis != VESTIS_NULLA)
            p->modus_vestis = VESTIS_STOLA;
        break;
    case ARCH_GENIUS:
            /* Spiritus personalis divinus — forma iuvenilis, oculi magni, genae rotundae */
        dir(&p->aetas, 0.05f, 0.9f);
        dir(&p->magnitudo_oculi, 0.85f, 0.5f);
        dir(&p->latitudo_nasi, 0.35f, 0.35f);
        dir(&p->longitudo_nasi, 0.35f, 0.35f);
        dir(&p->plenitudo_genae, 0.85f, 0.35f);
        p->modus_barbae = BARBA_NULLA;
        p->modus_mustaciorum = MUSTACIA_NULLA;
        dir(&p->crassitudo_labii_sup, 0.65f, 0.4f);
        dir(&p->crassitudo_labii_inf, 0.75f, 0.4f);
        break;
    case ARCH_FLAMEN:
            /* Sacerdos uni numini dedicatus — lineamenta elegantia, malae altae, compositus */
        dir(&p->latitudo_malae, 0.75f, 0.3f);
        dir(&p->color_cutis, 0.25f, 0.3f);
        dir(&p->longitudo_nasi, 0.65f, 0.3f);
        dir(&p->latitudo_nasi, 0.35f, 0.4f);
        dir(&p->angulus_anguli_oris, 0.5f, 0.3f);
        dir(&p->altitudo_frontis, 0.65f, 0.3f);
        if (p->modus_vestis != VESTIS_NULLA)
            p->modus_vestis = VESTIS_TOGA;
        p->modus_ornamenti = ORNAMENTUM_VITTA;
        break;
    case ARCH_FAUNUS:
            /* Rusticus spiritus silvestris, versipellis — asymmetricus, subrisus, scientes oculi */
        dir(&p->inclinatio_oculi, 0.65f, 0.3f);
        dir(&p->arcus_supercilii, 0.7f, 0.4f);
        dir(&p->angulus_anguli_oris, 0.65f, 0.5f);  /* subrisus */
        dir(&p->calor_cutis, 0.65f, 0.3f);
        dir(&p->densitas_barbae, 0.4f, 0.2f);
        p->modus_vestis = VESTIS_NULLA;   /* silvester nudus */
        break;
    default:
        break;
    }
}

void gens_applica(FaciesParametra* p, Gens gens) {
    if (!p)
        return;
    p->gens = gens;
    switch (gens) {
    case GENS_NYMPHARUM:
        p->acies_auris = 1.0f;
        dir(&p->inclinatio_oculi, 0.75f, 0.6f);
        dir(&p->longitudo_nasi, 0.55f, 0.3f);
        dir(&p->latitudo_nasi,  0.35f, 0.4f);
        dir(&p->altitudo_frontis, 0.65f, 0.4f);
        dir(&p->magnitudo_oculi, 0.7f, 0.4f);
        dir(&p->color_cutis, 0.15f, 0.4f);
        break;
    case GENS_PYGMAEORUM:
        /* Pygmaei: gens parva bellicosa — capita maiora proportione (puerilia),
         * facies rotundior, sed aetas plena. Non barbati ut nani Tolkieniani. */
        dir(&p->latitudo_faciei, 0.75f, 0.4f);
        dir(&p->altitudo_faciei, 0.45f, 0.3f);
        dir(&p->magnitudo_oculi, 0.7f, 0.4f);
        dir(&p->plenitudo_genae, 0.7f, 0.4f);
        dir(&p->latitudo_nasi, 0.60f, 0.3f);
        dir(&p->calor_cutis, 0.70f, 0.3f);
        break;
    case GENS_GIGANTUM:
        /* Gigantes plerumque dentes exsertos habent — inferi protrudentes (ex mandibula) */
        if (p->modus_dentium == DENTES_NULLI)
            p->modus_dentium = DENTES_CANINI_INFERI;
        p->magnitudo_dentis = fmaxf(p->magnitudo_dentis, 0.65f);
        dir(&p->crassitudo_supercilii, 0.85f, 0.6f);
        dir(&p->altitudo_supercilii, 0.3f, 0.5f);
        p->color_iridis_h = 0.1f;
        p->color_iridis_s = 0.8f;
            /* cutis viridis-ish: calor ad frigidus + HSL green — sed cutis formula
             * est h=0.03..0.08 — ergo adhibemus bioluminescentia low et post-shift */
        break;
    case GENS_PENATIUM:
        p->acies_auris = 0.6f;
        dir(&p->latitudo_faciei, 0.75f, 0.5f);
        dir(&p->plenitudo_genae, 0.8f, 0.5f);
        dir(&p->magnitudo_oculi, 0.75f, 0.4f);
        dir(&p->latitudo_nasi, 0.35f, 0.4f);
        break;
    case GENS_LARVARUM:
        dir(&p->color_cutis, 0.15f, 0.5f);
        dir(&p->calor_cutis, 0.15f, 0.5f);
        dir(&p->pondus_palpebrae, 0.75f, 0.5f);
        p->color_iridis_s = 0.15f;
            /* oculi concavi simulati per umbras */
        break;
    case GENS_FURIARUM:
        /* Furiae Romanae authenticae: oculi sanguinei cum sclera rubra, pupillae fissae,
         * cutis furens rubra. Cornua NON pertinent (illud Christianum-daemonicum est);
         * serpentes in coma traditionaliter, sed per venae_sclerae maximas et iris
         * ardentem significamus. */
        p->venae_sclerae = fmaxf(p->venae_sclerae, 0.85f);
        dir(&p->color_iridis_s, 0.95f, 0.7f);
        dir(&p->inclinatio_oculi, 0.75f, 0.5f);
        dir(&p->calor_cutis, 0.85f, 0.5f);
        dir(&p->arcus_supercilii, 0.85f, 0.6f);
        /* Dentes superi acuti saepe apud Furias */
        if (p->modus_dentium == DENTES_NULLI)
            p->modus_dentium = DENTES_CANINI_SUPERI;
        p->magnitudo_dentis = fmaxf(p->magnitudo_dentis, 0.55f);
        break;
    case GENS_SATYRORUM:
        /* Satyri/Fauni: cornicula parva ex fronte, nasus pressus, barba hirsuta */
        p->magnitudo_cornu = fmaxf(p->magnitudo_cornu, 0.25f);
        dir(&p->latitudo_nasi, 0.85f, 0.7f);
        dir(&p->longitudo_nasi, 0.75f, 0.5f);
        dir(&p->magnitudo_oculi, 0.7f, 0.4f);
        dir(&p->altitudo_supercilii, 0.7f, 0.4f);
        dir(&p->densitas_barbae, 0.6f, 0.3f);
        break;
    default:
        break;
    }
}

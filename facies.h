/* facies.h — FaciesParametra, ZonaeFaciei. */
#ifndef PORTRAIT_FACIES_H
#define PORTRAIT_FACIES_H

#include "commonia.h"
#include "homuncio.h"

/* Omnia parametra est [0,1] aut enum. */
typedef struct {
    /* --- Identitas --- */
    uint64_t      semen;
    float         tempus;
    FormaFaciei   forma_faciei;
    Gens          gens;
    Archetypum    archetypum;

    /* --- Demographica --- */
    float aetas;
    float color_cutis;
    float calor_cutis;
    float masculinitas;

    /* --- Geometria faciei --- */
    float latitudo_faciei, altitudo_faciei;
    float latitudo_malae, quadratura_malae;
    float plenitudo_genae;
    float rima_menti;

    /* --- Oculi --- */
    float magnitudo_oculi, distantia_oculorum, inclinatio_oculi;
    float pondus_palpebrae;
    float plica_epicanthica;
    float magnitudo_iridis;
    float color_iridis_h, color_iridis_s;
    float venae_sclerae;

    /* --- Supercilia --- */
    float altitudo_supercilii, arcus_supercilii;
    float crassitudo_supercilii, longitudo_supercilii;
    float intervallum_supercilii;

    /* --- Nasus --- */
    float latitudo_nasi, longitudo_nasi;
    float altitudo_pontis, latitudo_pontis;
    float forma_apicis;
    float curvatura_nasi;
    float dilatatio_narium;

    /* --- Os --- */
    float latitudo_oris, altitudo_oris;
    float crassitudo_labii_sup, crassitudo_labii_inf;
    float arcus_cupidinis;
    float profunditas_philtri;
    float angulus_anguli_oris;   /* -1..+1 */

    /* --- Aures --- */
    float magnitudo_auris, protrusio_auris;
    float magnitudo_lobi;
    float acies_auris;

    /* --- Coma --- */
    ModusComae  modus_comae;
    float       volumen_comae, longitudo_comae;
    float       altitudo_frontis, recessus_frontis;
    float       color_comae_h, color_comae_s, color_comae_v;
    float       canitudo;

    /* --- Barba --- */
    ModusBarbae       modus_barbae;
    float             densitas_barbae;
    ModusMustaciorum  modus_mustaciorum;

    /* --- Vestis --- */
    ModusVestis       modus_vestis;
    float             color_vestis_h, color_vestis_s, color_vestis_v;
    float             color_ornamenti_h;   /* color clavi/fibulae/fold */

    /* --- Ornamenta capitis --- */
    ModusOrnamenti    modus_ornamenti;
    float             color_capitis_h;     /* color fasciae/pilei — cloth/leaf */

    /* --- Asymmetria et heterochromia oculorum --- */
    float             heterochromia;        /* 0 = nulla, >0.5 = activa */
    float             color_iridis2_h;
    float             color_iridis2_s;
    float             oculi_asymmetria;     /* subtilis differentia magnitudinis */

    /* --- Cutis --- */
    float densitas_lentiginorum;
    float profunditas_rugarum;
    float numerus_cicatricum;
    float visibilitas_pororum;

    /* --- Variantes phantasticae --- */
    float magnitudo_dentis;
    float magnitudo_cornu;
    float bioluminescentia;
} FaciesParametra;

/* Zonae faciei: spatia coordinatarum derivata ex parametris. */
typedef struct {
    vec2  centrum_faciei;
    float lat_faciei, alt_faciei;

    struct {
        vec2  sin, dex;
        float y;
        float distantia;
        float r_iridis;
        float r_oculi_x, r_oculi_y;
        float inclinatio;
    } oculi;

    struct {
        vec2 centrum;
        float lat, alt;
        float pontis_lat;
    } nasus;

    struct {
        vec2 centrum;
        float lat, alt;
        float labium_sup_t;
        float labium_inf_t;
    } os;

    struct {
        float y;
        vec2 sin, dex;
        float lat, alt;
        float acies;
    } aures;

    struct {
        vec2 centrum;
        float r;
    } mentum;

    struct {
        float y_frons, y_culmen;
        float recessus;
    } frons;

    struct {
        float y_sup, y_inf;
    } cervix;

    struct {
        vec2 centrum;
        float lat, alt;
    } supercilia_sin, supercilia_dex;

} ZonaeFaciei;

/* Computat zonas ex parametris. */
ZonaeFaciei zonae_computa(const FaciesParametra* p, int w, int h);

/* SDF silhouettae faciei (d < 0 intus) in spatio pixelum */
float sdf_contura_faciei(vec2 p, const FaciesParametra* par, const ZonaeFaciei* z);

/* Mutatio per nomen clavis (returns 1 si agnotum, 0 si ignotum) */
int facies_pone_clavem(FaciesParametra* p, const char* clavis, float valor);

/* Initialisatio defaulta — vulto neutrali humano medii aetatis */
void facies_default(FaciesParametra* p);

/* Generatio ex semine: sortitur omnia parametra in distributionibus neutris. */
void facies_genera(FaciesParametra* p, uint64_t semen);

#endif

/* lineamenta.h — oculi, supercilia, nasus, os, aures.
 *
 * Omnia reddita per SDF + cobertura. */
#ifndef PORTRAIT_LINEAMENTA_H
#define PORTRAIT_LINEAMENTA_H

#include "tabula.h"
#include "facies.h"

typedef struct {
    Color cutis;
    Color cutis_umbra;
    Color cutis_lux;
    Color iridis;
    Color labia;
    Color sclera;
    Color os_interior;
    Color comae;
    Color comae_obscura;
    Color supercilia;
    Color lineae;           /* colores linearum pro cartoon */
} PalettaFaciei;

/* Derivat palettam ex parametris */
PalettaFaciei paletta_computa(const FaciesParametra* p);

/* Redditio singularium lineamentorum */
void redde_aures      (Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);
void redde_oculos     (Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);
void redde_supercilia (Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);
void redde_nasum      (Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);
void redde_os         (Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);
void redde_cornua     (Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);

#endif

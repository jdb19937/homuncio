/* coma.h — reddere comam (massa + fila + barba) */
#ifndef PORTRAIT_COMA_H
#define PORTRAIT_COMA_H

#include "tabula.h"
#include "facies.h"
#include "lineamenta.h"    /* PalettaFaciei */

/* Redde massam comae + fila (ante faciem, sed sub oculis in sequentia pipelinis) */
void redde_coma_massa (Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);
void redde_coma_fila  (Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);
void redde_barba      (Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);
void redde_mustacia   (Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);

#endif

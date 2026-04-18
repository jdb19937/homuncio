/* cutis.h — strata texturae cutis. */
#ifndef PORTRAIT_CUTIS_H
#define PORTRAIT_CUTIS_H

#include "tabula.h"
#include "facies.h"
#include "lineamenta.h"  /* PalettaFaciei */

/* Pingit faciem plenam cutim (ellipsis distorta) + umbras subtiles */
void redde_cutis_base(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);

/* Applicat texturam ex pororum, rugis, lentiginibus, cicatricibus, etc. */
void redde_cutis_textura(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);

/* Cicatrices lineares */
void redde_cicatrices(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);

/* Rugae pro aetate */
void redde_rugas(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);

/* Lentigines + pigmenti variationes */
void redde_lentigines(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);

/* Dentes exserti (pro orcis/daemonibus) */
void redde_dentes(Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z, const PalettaFaciei* col);

#endif

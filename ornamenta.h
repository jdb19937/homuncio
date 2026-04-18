/* ornamenta.h — ornamenta capitis (corona laurea, galea, vitta, pileus, diadema) */
#ifndef PORTRAIT_ORNAMENTA_H
#define PORTRAIT_ORNAMENTA_H

#include "tabula.h"
#include "facies.h"
#include "lineamenta.h"

void redde_ornamenta_capitis(
    Tabula* t, const FaciesParametra* p,
    const ZonaeFaciei* z, const PalettaFaciei* col
);

#endif

/* vestis.h — reddit vestimenta Romana super umeros bustique. */
#ifndef PORTRAIT_VESTIS_H
#define PORTRAIT_VESTIS_H

#include "tabula.h"
#include "facies.h"
#include "lineamenta.h"

void redde_vestis(
    Tabula* t, const FaciesParametra* p, const ZonaeFaciei* z,
    const PalettaFaciei* col
);

#endif

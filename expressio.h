/* expressio.h — deltae expressionis super faciem neutralem. */
#ifndef PORTRAIT_EXPRESSIO_H
#define PORTRAIT_EXPRESSIO_H

#include "homuncio.h"
#include "facies.h"

/* Applicat expressionem ad FaciesParametra (pondus 0..1). */
void facies_applica_expressionem(FaciesParametra* p, const Expressio* e, float pondus);

/* Nictus oculorum ex tempus — redit apertionem [0..1] pro sinistro et dextro.
 * Duo nictus in tempore periodico: ad tempus ≈ 0.1 et 0.6 — intervalla brevia. */
float expressio_nictus(float tempus);

#endif

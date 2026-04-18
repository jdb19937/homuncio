/* archetypum.h — distributiones prior + modificatores gentis. */
#ifndef PORTRAIT_ARCHETYPUM_H
#define PORTRAIT_ARCHETYPUM_H

#include "facies.h"

/* Applicat archetypum (biasat parametra) et gentem (modifiers phantastici). */
void archetypum_applica(FaciesParametra* p, Archetypum arch);
void gens_applica       (FaciesParametra* p, Gens gens);

#endif

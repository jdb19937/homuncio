/* modus.h — pipelines styli */
#ifndef PORTRAIT_MODUS_H
#define PORTRAIT_MODUS_H

#include "tabula.h"
#include "homuncio.h"

/* Applicat stilum ad tabulam completam.  Geometria iam reddita est in t;
 * haec functio applicat transformationem post-redditionis specificam styli
 * (posterizatio pro cartoon, scanlineae pro 8-bit, etc). */
void modus_applica(Tabula* t, ModusArtis modus, float tempus);

#endif

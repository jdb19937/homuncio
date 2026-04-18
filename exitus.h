/* exitus.h — scriptor PPM (P6 binarius). */
#ifndef PORTRAIT_EXITUS_H
#define PORTRAIT_EXITUS_H

#include "commonia.h"

/* Scribit PPM P6 ex capsam RGBA8 (discardo canalis A).
 * Redit 0 pro successu, -1 pro errore. */
int exitus_scribe_ppm(const char* via, const uint8_t* rgba, int w, int h);

#endif

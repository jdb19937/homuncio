/* tumultus.h — strepitus valoris (value noise) + FBM + distortio. */
#ifndef PORTRAIT_TUMULTUS_H
#define PORTRAIT_TUMULTUS_H

#include "commonia.h"

/* Strepitus valoris 2D — deterministicus, continuus, periodus ~2^32.
 * Exitus ∈ [-1, 1]. */
float tumultus2(float x, float y);

/* Strepitus valoris cum semen — permittit campos diversos ex diversis seminibus */
float tumultus2_s(float x, float y, uint32_t semen);

/* FBM: summa octavarum cum lucrum (persistence) et lucrum frequentiae fixum 2.0 */
float fbm(float x, float y, int octavae, float lucrum);
float fbm_s(float x, float y, int octavae, float lucrum, uint32_t semen);

/* Distortio regionis: vec2 campus ex duobus strepitibus */
vec2  distortio_regionis(vec2 p, float vis, uint32_t semen);

/* Punctum ut strepitus in [0, 1] */
static inline float tumultus01(float x, float y, uint32_t semen) {
    return tumultus2_s(x, y, semen) * 0.5f + 0.5f;
}

/* Fluxus directionis (ut unit-vector) ex gradiente fbm */
vec2  fluxus_directio(float x, float y, uint32_t semen);

/* Worley-like: distantia ad nodum proximum (cellae quadratae de 1.0) */
float worley2(float x, float y, uint32_t semen);

#endif

/* sors.h — PRNG (xorshift64) + distributiones utiles */
#ifndef PORTRAIT_SORS_H
#define PORTRAIT_SORS_H

#include "commonia.h"

typedef struct {
    uint64_t status;
}Sors;

/* Initialisat sortem ex semine; nullo semine admisso, constans defaulta usa. */
Sors     sors_inita(uint64_t semen);

/* Numerus primarius [1, 2^64 - 1] */
uint64_t sors_proximus(Sors* s);

/* [0, 1) */
float    sors_f32(Sors* s);

/* [inf, sup) */
float    sors_spatium(Sors* s, float inf, float sup);

/* Distributio normalis (Box-Muller) */
float    sors_gauss(Sors* s, float mu, float sigma);

/* Electio per pondera [0..n-1] */
int      sors_ponderatus(Sors* s, const float* pondera, int n);

/* Mixtio deterministica paratri: ad ±amplitudo circum medium, nondeterministica finali */
float    sors_circum(Sors* s, float medium, float amplitudo);

/* Permutatio bitium — adhibetur ut semen dictam in seminem pro lineamento generet */
uint64_t sors_hash(uint64_t x);

/* Substrat: producit sortem novam ex duobus seminibus deterministice */
Sors     sors_deriva(uint64_t semen_primum, uint64_t dictum);

/* Conversio ex char* -> uint64_t (hash deterministicus) — pro usu CLI */
uint64_t sors_hash_nominis(const char* nomen);

#endif

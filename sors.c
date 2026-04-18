#include "sors.h"

/* xorshift64: Marsaglia 2003.  Periodus 2^64-1. */
Sors sors_inita(uint64_t semen) {
    Sors s;
    if (semen == 0)
        semen = 0x9E3779B97F4A7C15ULL;
    s.status = semen;
    /* "Warmup" ut bitia primae iterationis melius misceantur */
    (void) sors_proximus(&s);
    (void) sors_proximus(&s);
    return s;
}

uint64_t sors_proximus(Sors* s) {
    uint64_t x = s->status;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    s->status = x ? x : 0x9E3779B97F4A7C15ULL;
    return s->status;
}

float sors_f32(Sors* s) {
    /* Sumimus 24 bita summa pro mantissa (single precision) */
    uint64_t r = sors_proximus(s);
    uint32_t m = (uint32_t)(r >> 40);   /* 24 bita */
    return (float) m * (1.0f / (float)(1u << 24));
}

float sors_spatium(Sors* s, float inf, float sup) {
    return inf + (sup - inf) * sors_f32(s);
}

float sors_gauss(Sors* s, float mu, float sigma) {
    /* Box-Muller polaris: clamp u1 > 0 ne log diverget */
    float u1 = sors_f32(s);
    float u2 = sors_f32(s);
    if (u1 < 1e-7f)
        u1 = 1e-7f;
    float mag = sigma * sqrtf(-2.0f * logf(u1));
    return mu + mag * cosf(2.0f * PORTRAIT_PI * u2);
}

int sors_ponderatus(Sors* s, const float* pondera, int n) {
    if (n <= 0)
        return 0;
    float summa = 0.0f;
    for (int i = 0; i < n; i++)
        summa += pondera[i] > 0.0f ? pondera[i] : 0.0f;
    if (summa <= 0.0f)
        return 0;
    float r = sors_f32(s) * summa;
    float cum = 0.0f;
    for (int i = 0; i < n; i++) {
        cum += pondera[i] > 0.0f ? pondera[i] : 0.0f;
        if (r < cum)
            return i;
    }
    return n - 1;
}

float sors_circum(Sors* s, float medium, float amplitudo) {
    return medium + amplitudo * (2.0f * sors_f32(s) - 1.0f);
}

uint64_t sors_hash(uint64_t x) {
    /* SplitMix64 finalisatio */
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x ? x : 1ULL;
}

Sors sors_deriva(uint64_t semen_primum, uint64_t dictum) {
    return sors_inita(sors_hash(semen_primum ^ sors_hash(dictum + 0x9E3779B97F4A7C15ULL)));
}

uint64_t sors_hash_nominis(const char* nomen) {
    uint64_t h = 1469598103934665603ULL;
    if (!nomen)
        return h;
    for (const unsigned char* p = (const unsigned char*)nomen; *p; p++) {
        h ^= (uint64_t)(*p);
        h *= 1099511628211ULL;
    }
    return sors_hash(h);
}

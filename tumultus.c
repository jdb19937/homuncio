#include "tumultus.h"

/* Hash entier 2D -> [0,1) deterministicus */
static inline float hash2(int x, int y, uint32_t semen) {
    uint32_t h = (uint32_t)x * 73856093u ^ (uint32_t)y * 19349663u ^ semen * 2654435761u;
    h ^= h >> 13;
    h *= 0x5bd1e995u;
    h ^= h >> 15;
    return (float)(h & 0x00FFFFFFu) * (1.0f / 16777216.0f);
}

float tumultus2_s(float x, float y, uint32_t semen) {
    int xi = (int) floorf(x);
    int yi = (int) floorf(y);
    float fx = x - (float)xi;
    float fy = y - (float)yi;
    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uy = fy * fy * (3.0f - 2.0f * fy);

    float a = hash2(xi,     yi,     semen);
    float b = hash2(xi + 1, yi,     semen);
    float c = hash2(xi,     yi + 1, semen);
    float d = hash2(xi + 1, yi + 1, semen);

    float ab = a + (b - a) * ux;
    float cd = c + (d - c) * ux;
    float v  = ab + (cd - ab) * uy;
    return v * 2.0f - 1.0f;
}

float tumultus2(float x, float y) { return tumultus2_s(x, y, 1337u); }

float fbm_s(float x, float y, int octavae, float lucrum, uint32_t semen) {
    if (octavae < 1)
        octavae = 1;
    if (octavae > 8)
        octavae = 8;
    float sum = 0.0f;
    float amp = 1.0f;
    float frq = 1.0f;
    float norm = 0.0f;
    for (int i = 0; i < octavae; i++) {
        sum += amp * tumultus2_s(x * frq, y * frq, semen + (uint32_t)i * 17u);
        norm += amp;
        amp *= lucrum;
        frq *= 2.0f;
    }
    return sum / (norm + 1e-10f);
}

float fbm(float x, float y, int octavae, float lucrum) {
    return fbm_s(x, y, octavae, lucrum, 1337u);
}

vec2 distortio_regionis(vec2 p, float vis, uint32_t semen) {
    float dx = fbm_s(p.x, p.y, 4, 0.5f, semen);
    float dy = fbm_s(p.x + 13.7f, p.y - 9.1f, 4, 0.5f, semen ^ 0xA5A5A5A5u);
    return v2(dx * vis, dy * vis);
}

vec2 fluxus_directio(float x, float y, uint32_t semen) {
    const float e = 0.5f;
    float dx = fbm_s(x + e, y, 3, 0.5f, semen) - fbm_s(x - e, y, 3, 0.5f, semen);
    float dy = fbm_s(x, y + e, 3, 0.5f, semen) - fbm_s(x, y - e, 3, 0.5f, semen);
    float L = sqrtf(dx*dx + dy*dy) + 1e-10f;
    return v2(-dy / L, dx / L);  /* perpendicularis ad gradientem ⇒ sequitur contours */
}

float worley2(float x, float y, uint32_t semen) {
    int xi = (int) floorf(x);
    int yi = (int) floorf(y);
    float fx = x - (float)xi;
    float fy = y - (float)yi;
    float best = 1e9f;
    for (int j = -1; j <= 1; j++) {
        for (int i = -1; i <= 1; i++) {
            /* ponit nodum cellae in positione ex hash */
            float nx = (float)i + hash2(xi+i, yi+j, semen) - fx;
            float ny = (float)j + hash2(xi+i, yi+j, semen ^ 0xdeadbeefu) - fy;
            float d = nx*nx + ny*ny;
            if (d < best)
                best = d;
        }
    }
    return sqrtf(best);
}

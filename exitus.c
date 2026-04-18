#include "exitus.h"
#include <stdio.h>
#include <stdlib.h>

int exitus_scribe_ppm(const char* via, const uint8_t* rgba, int w, int h) {
    if (!via || !rgba || w <= 0 || h <= 0)
        return -1;
    FILE* f = fopen(via, "wb");
    if (!f)
        return -1;
    /* Caput PPM P6 */
    if (fprintf(f, "P6\n%d %d\n255\n", w, h) < 0) {
        fclose(f);
        return -1;
    }
    /* Corpus: RGB, canalis alpha discartatur */
    size_t n = (size_t)w * (size_t)h;
    uint8_t* buf = (uint8_t*) malloc(n * 3u);
    if (!buf) {
        fclose(f);
        return -1;
    }
    for (size_t i = 0; i < n; i++) {
        buf[i*3+0] = rgba[i*4+0];
        buf[i*3+1] = rgba[i*4+1];
        buf[i*3+2] = rgba[i*4+2];
    }
    size_t written = fwrite(buf, 1, n * 3u, f);
    free(buf);
    fclose(f);
    return (written == n * 3u) ? 0 : -1;
}

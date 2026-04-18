/* matricula.c — generat 8×8 matricem imaginum personarum cum parametris fortuitis.
 *
 * Usus:
 *   ./matricula                      (semen ex tempore, exitus = matricula.ppm)
 *   ./matricula -s 42 -o mat.ppm
 *   ./matricula --dim 4              (4×4 pro testando)
 *   ./matricula --tempus 0.2         (omnia portraita cum eodem parametro temporis)
 */

#include "homuncio.h"
#include "sors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void usus(const char* argv0) {
    fprintf(
        stderr,
        "usus: %s [optiones]\n"
        "\n"
        "Componit matricem NxN imaginum personarum cum parametris fortuitis.\n"
        "\n"
        "  -o, --output <via>   via exitus PPM                   [default: matricula.ppm]\n"
        "  -s, --semen <valor>  semen radix (integer vel nomen)  [default: tempore]\n"
        "  -d, --dim <N>        magnitudo matricis (NxN)         [default: 8]\n"
        "  -t, --tempus <float> parametrum animationis pro omnibus [default: 0.0]\n"
        "  -h, --help           ostende hunc textum\n",
        argv0
    );
}

extern int exitus_scribe_ppm(const char*, const uint8_t*, int, int);

static const char* arg_val(int argc, char** argv, int* i, const char* name) {
    if (*i + 1 >= argc) {
        fprintf(stderr, "optio %s requirit valorem\n", name);
        exit(1);
    }
    (*i)++;
    return argv[*i];
}

int main(int argc, char** argv) {
    const char* via    = "matricula.ppm";
    uint64_t    semen  = 0;         /* 0 ⇒ ex tempore */
    int         dim    = 8;
    float       tempus = 0.0f;
    int         semen_datum = 0;

    for (int i = 1; i < argc; i++) {
        const char* a = argv[i];
        if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
            usus(argv[0]);
            return 0;
        }else if (strcmp(a, "-o") == 0 || strcmp(a, "--output") == 0) {
            via = arg_val(argc, argv, &i, "--output");
        } else if (strcmp(a, "-s") == 0 || strcmp(a, "--semen") == 0) {
            const char* v = arg_val(argc, argv, &i, "--semen");
            if (v[0] >= '0' && v[0] <= '9')
                semen = (uint64_t) strtoull(v, NULL, 10);
            else
                semen = sors_hash_nominis(v);
            semen_datum = 1;
        } else if (strcmp(a, "-d") == 0 || strcmp(a, "--dim") == 0) {
            dim = atoi(arg_val(argc, argv, &i, "--dim"));
            if (dim < 1 || dim > 32) {
                fprintf(stderr, "dim debet esse in [1, 32]\n");
                return 1;
            }
        } else if (strcmp(a, "-t") == 0 || strcmp(a, "--tempus") == 0) {
            tempus = (float) atof(arg_val(argc, argv, &i, "--tempus"));
        } else {
            fprintf(stderr, "argumentum ignotum: %s\n", a);
            fprintf(stderr, "proba '%s --help' pro auxilio.\n", argv[0]);
            return 1;
        }
    }

    if (!semen_datum) {
        semen = (uint64_t) time(NULL) ^ ((uint64_t) clock() << 17);
    }

    int cell = PORTRAIT_LATITUDO;  /* 128 */
    int W = cell * dim;
    int H = cell * dim;

    uint8_t* mat  = (uint8_t*) calloc((size_t) W * (size_t) H * 4u, 1);
    uint8_t* cbuf = (uint8_t*) malloc((size_t) cell * cell * 4u);
    if (!mat || !cbuf) {
        fprintf(stderr, "allocatio frustra\n");
        return 1;
    }

    Sors s = sors_inita(semen);

    /* Septem expressiones (neutrum + 7 emoti — eligimus 8 pro varietate) */
    Expressio expr_tab[8] = {
        EXPR_NEUTRUM, EXPR_LAETUM, EXPR_TRISTE, EXPR_IRATUM,
        EXPR_MIRANS, EXPR_TIMIDUM, EXPR_FASTIDIOSUM, EXPR_CONTEMPTUM,
    };

    fprintf(
        stderr, "matricula %dx%d (semen: %llu) ... ", dim, dim,
        (unsigned long long) semen
    );

    int done = 0;
    for (int r = 0; r < dim; r++) {
        for (int c = 0; c < dim; c++) {
            /* Eligit parametra fortuita */
            uint64_t subsemen = sors_proximus(&s);
            Archetypum arch   = (Archetypum)(sors_proximus(&s) % ARCH_NUMERUS);
            Gens gens         = (Gens)      (sors_proximus(&s) % GENS_NUMERUS);

            /* Modus artis ponderatus — monochrome raro (ATRAMENTUM, NIGER) */
            float pond_modi[MODUS_NUMERUS];
            pond_modi[MODUS_CARTOON]       = 0.30f;
            pond_modi[MODUS_PIXEL]         = 0.12f;
            pond_modi[MODUS_ATRAMENTUM]    = 0.05f;   /* rarus */
            pond_modi[MODUS_PICTUM]        = 0.18f;
            pond_modi[MODUS_LUDICRUM_VIII] = 0.10f;
            pond_modi[MODUS_ANIME]         = 0.20f;
            pond_modi[MODUS_NIGER]         = 0.05f;   /* rarus */
            ModusArtis modus = (ModusArtis) sors_ponderatus(&s, pond_modi, MODUS_NUMERUS);

            /* Fx: solum ex fx coloratis (excludit halftone + FS dither qui monochromum producunt) */
            static const PostEffectus fx_chromatici[] = {
                FX_VIGNETTA, FX_GRANUM, FX_SCANLINEAE_CRT,
                FX_ABER_CHROMATIS, FX_DITHERING_BAYER,
                FX_POSTERIZATIO, FX_LINEAE_PROMINENTES, FX_NITOR
            };
            int n_fx = (int)(sizeof(fx_chromatici) / sizeof(fx_chromatici[0]));
            PostEffectus fx = sors_f32(&s) < 0.35f
                ? fx_chromatici[sors_proximus(&s) % (unsigned)n_fx]
                : FX_NULLUS;
            float vis_fx      = sors_spatium(&s, 0.2f, 0.7f);
            /* Expressio: ~70% habet aliquam expressionem non-neutralem */
            int with_expr     = sors_f32(&s) < 0.7f;
            int expr_idx      = sors_proximus(&s) % 8;
            Expressio expr    = expr_tab[expr_idx];
            float pondus      = with_expr ? sors_spatium(&s, 0.5f, 1.0f) : 0.0f;

            Imago* im = imago_nova(subsemen, arch, gens);
            if (!im)
                continue;
            if (pondus > 0.001f)
                imago_pone_expressionem(im, expr, pondus);

            imago_redde(im, cbuf, modus, fx, vis_fx, tempus);
            imago_dele(im);

            /* Copia in positionem matricis */
            int x0 = c * cell;
            int y0 = r * cell;
            for (int y = 0; y < cell; y++) {
                for (int x = 0; x < cell; x++) {
                    int src = (y * cell + x) * 4;
                    int dst = ((y0 + y) * W + (x0 + x)) * 4;
                    mat[dst + 0] = cbuf[src + 0];
                    mat[dst + 1] = cbuf[src + 1];
                    mat[dst + 2] = cbuf[src + 2];
                    mat[dst + 3] = 255;
                }
            }

            done++;
            if ((done % 8) == 0) {
                fputc('.', stderr);
                fflush(stderr);
            }
        }
    }
    fprintf(stderr, " (%d imagines) peractum\n", done);

    int rv = exitus_scribe_ppm(via, mat, W, H);
    free(mat);
    free(cbuf);

    if (rv != 0) {
        fprintf(stderr, "scriptio PPM frustra: %s\n", via);
        return 1;
    }
    fprintf(stderr, "scripta: %s (%dx%d)\n", via, W, H);
    return 0;
}

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

            /* Modus artis ponderatus — monochrome rarissime (ATRAMENTUM, NIGER) */
            float pond_modi[MODUS_NUMERUS];
            pond_modi[MODUS_CARTOON]       = 0.32f;
            pond_modi[MODUS_PIXEL]         = 0.13f;
            pond_modi[MODUS_ATRAMENTUM]    = 0.02f;   /* rarissimus */
            pond_modi[MODUS_PICTUM]        = 0.20f;
            pond_modi[MODUS_LUDICRUM_VIII] = 0.11f;
            pond_modi[MODUS_ANIME]         = 0.20f;
            pond_modi[MODUS_NIGER]         = 0.02f;   /* rarissimus */
            ModusArtis modus = (ModusArtis) sors_ponderatus(&s, pond_modi, MODUS_NUMERUS);

            /* Fx chromatici (excludit halftone + FS dither qui monochromum producunt) */
            static const PostEffectus fx_chromatici[] = {
                FX_VIGNETTA, FX_GRANUM, FX_SCANLINEAE_CRT,
                FX_ABER_CHROMATIS, FX_DITHERING_BAYER,
                FX_POSTERIZATIO, FX_LINEAE_PROMINENTES, FX_NITOR
            };
            int n_chrom = (int)(sizeof(fx_chromatici) / sizeof(fx_chromatici[0]));
            /* Fx rari — monochromatici, solum ut aberratio singularis */
            static const PostEffectus fx_rari[] = {
                FX_HALFTONE, FX_DITHERING_FS
            };
            int n_rari = (int)(sizeof(fx_rari) / sizeof(fx_rari[0]));

            /* Vis fx bimodalis: 75% moderata [0.25, 0.6], 25% fortis [0.6, 0.9] */
            #define SAMPLE_VIS(SS) (sors_f32(SS) < 0.25f \
                ? sors_spatium(SS, 0.6f, 0.9f)          \
                : sors_spatium(SS, 0.25f, 0.6f))

            /* Distributio fx: 45% nullus, 40% unum, 12.5% duo, 2.5% aberratio rara */
            PostEffectus fx1 = FX_NULLUS, fx2 = FX_NULLUS;
            float vis1 = 0.0f, vis2 = 0.0f;
            float fx_roll = sors_f32(&s);
            if (fx_roll < 0.025f) {
                /* Aberratio rara: halftone aut FS dither (exclusivus) */
                fx1  = fx_rari[sors_proximus(&s) % (unsigned)n_rari];
                vis1 = SAMPLE_VIS(&s);
            } else if (fx_roll < 0.150f) {
                /* Duo fx chromatici diversi */
                int i = (int)(sors_proximus(&s) % (unsigned)n_chrom);
                int j;
                do {
                    j = (int)(sors_proximus(&s) % (unsigned)n_chrom);
                }while (j == i);
                fx1  = fx_chromatici[i];
                fx2  = fx_chromatici[j];
                vis1 = SAMPLE_VIS(&s);
                vis2 = SAMPLE_VIS(&s);
            } else if (fx_roll < 0.550f) {
                /* Unum fx chromaticum */
                fx1  = fx_chromatici[sors_proximus(&s) % (unsigned)n_chrom];
                vis1 = SAMPLE_VIS(&s);
            }
            #undef SAMPLE_VIS

            /* Expressio: ~80% habet aliquam expressionem non-neutralem, fortior */
            int with_expr     = sors_f32(&s) < 0.80f;
            int expr_idx      = sors_proximus(&s) % 8;
            Expressio expr    = expr_tab[expr_idx];
            float pondus      = with_expr ? sors_spatium(&s, 0.6f, 1.0f) : 0.0f;

            Imago* im = imago_nova(subsemen, arch, gens);
            if (!im)
                continue;
            if (pondus > 0.001f)
                imago_pone_expressionem(im, expr, pondus);

            /* Rara bioluminescentia (~5%): glow conspicuus */
            if (sors_f32(&s) < 0.05f) {
                imago_pone_parametrum(
                    im, "bioluminescentia",
                    sors_spatium(&s, 0.6f, 1.0f)
                );
            }

            /* Outlier aetatis / rugarum / cicatricum (~8%) */
            if (sors_f32(&s) < 0.08f) {
                int pick = (int)(sors_proximus(&s) % 3u);
                if (pick == 0) {
                    /* iuvenis aut senex */
                    float ae = sors_f32(&s) < 0.5f
                        ? sors_spatium(&s, 0.0f, 0.15f)
                        : sors_spatium(&s, 0.85f, 1.0f);
                    imago_pone_parametrum(im, "aetas", ae);
                } else if (pick == 1) {
                    imago_pone_parametrum(
                        im, "profunditas_rugarum",
                        sors_spatium(&s, 0.7f, 1.0f)
                    );
                } else {
                    imago_pone_parametrum(
                        im, "numerus_cicatricum",
                        sors_spatium(&s, 0.5f, 1.0f)
                    );
                }
            }

            /* Exotica capilli/oculi (~3%): hue vividum ex gamut saturato */
            if (sors_f32(&s) < 0.03f) {
                static const float hues_exot[] = {
                    0.75f, 0.95f, 0.33f, 0.55f, 0.85f
                };
                int nh = (int)(sizeof(hues_exot) / sizeof(hues_exot[0]));
                float hue_c = hues_exot[sors_proximus(&s) % (unsigned)nh];
                float hue_i = hues_exot[sors_proximus(&s) % (unsigned)nh];
                imago_pone_parametrum(im, "color_comae_h",  hue_c);
                imago_pone_parametrum(
                    im, "color_comae_s",
                    sors_spatium(&s, 0.75f, 1.0f)
                );
                imago_pone_parametrum(im, "color_iridis_h", hue_i);
                imago_pone_parametrum(
                    im, "color_iridis_s",
                    sors_spatium(&s, 0.75f, 1.0f)
                );
            }

            /* Coma extrema (~5%): calvus aut crinis longissima */
            if (sors_f32(&s) < 0.05f) {
                if (sors_f32(&s) < 0.5f) {
                    imago_pone_parametrum(im, "volumen_comae",   0.0f);
                    imago_pone_parametrum(im, "longitudo_comae", 0.0f);
                } else {
                    imago_pone_parametrum(im, "volumen_comae",   1.0f);
                    imago_pone_parametrum(im, "longitudo_comae", 1.0f);
                }
            }

            imago_redde_fx2(im, cbuf, modus, fx1, vis1, fx2, vis2, tempus);
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

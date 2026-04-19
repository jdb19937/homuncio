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
            /* Archetypum ponderatus: favet vīsuāliter distinctōs (senex, puer, silvester)
             * super medianōs (LAR/IANUS/FLAMEN/AUGUR qui biased similiter) */
            float pond_arch[ARCH_NUMERUS];
            pond_arch[ARCH_LAR]      = 0.08f;
            pond_arch[ARCH_MANES]    = 0.14f;   /* senex distinctus */
            pond_arch[ARCH_LEMUR]    = 0.12f;   /* luridus distinctus */
            pond_arch[ARCH_IANUS]    = 0.07f;
            pond_arch[ARCH_AUGUR]    = 0.09f;
            pond_arch[ARCH_QUIRINUS] = 0.12f;   /* cicatricatus distinctus */
            pond_arch[ARCH_CARNA]    = 0.10f;
            pond_arch[ARCH_GENIUS]   = 0.13f;   /* puer distinctus */
            pond_arch[ARCH_FLAMEN]   = 0.07f;
            pond_arch[ARCH_FAUNUS]   = 0.08f;
            Archetypum arch = (Archetypum) sors_ponderatus(&s, pond_arch, ARCH_NUMERUS);
            /* Gens: favet phantasticas ut varietas visibilis sit; humani 25%, alii 75% */
            float pond_gens[GENS_NUMERUS];
            pond_gens[GENS_HUMANA]     = 0.25f;
            pond_gens[GENS_NYMPHARUM]  = 0.10f;
            pond_gens[GENS_PYGMAEORUM] = 0.10f;
            pond_gens[GENS_GIGANTUM]   = 0.10f;
            pond_gens[GENS_PENATIUM]   = 0.10f;
            pond_gens[GENS_LARVARUM]   = 0.10f;
            pond_gens[GENS_FURIARUM]   = 0.12f;
            pond_gens[GENS_SATYRORUM]  = 0.13f;
            Gens gens = (Gens) sors_ponderatus(&s, pond_gens, GENS_NUMERUS);

            /* Modus artis ponderatus — monochrome rarissime (ATRAMENTUM, NIGER) */
            float pond_modi[MODUS_NUMERUS];
            pond_modi[MODUS_COMICUS]       = 0.32f;
            pond_modi[MODUS_TESSELLATUS]   = 0.13f;
            pond_modi[MODUS_ATRAMENTUM]    = 0.02f;   /* rarissimus */
            pond_modi[MODUS_PICTUM]        = 0.20f;
            pond_modi[MODUS_LUDICRUM_VIII] = 0.11f;
            pond_modi[MODUS_ORIENTALIS]    = 0.20f;
            pond_modi[MODUS_NIGER]         = 0.02f;   /* rarissimus */
            ModusArtis modus = (ModusArtis) sors_ponderatus(&s, pond_modi, MODUS_NUMERUS);

            /* Fx pro figura (fg): conservative — non obliteret vultum */
            static const PostEffectus fx_fg_set[] = {
                FX_VIGNETTA, FX_GRANUM, FX_SCANLINEAE_CRT,
                FX_ABER_CHROMATIS, FX_DITHERING_BAYER,
                FX_POSTERIZATIO, FX_LINEAE_PROMINENTES, FX_NITOR,
                FX_PATINA, FX_FRESCO, FX_AURUM, FX_RIMAE
            };
            int n_fg = (int)(sizeof(fx_fg_set) / sizeof(fx_fg_set[0]));

            /* Fx bg audax — ponderatus ut dramatici saepius eligantur super subtiles.
             * Pondus altior → saepius eligitur. */
            static const PostEffectus fx_bg_audax[]   = {
                FX_HALFTONE, FX_MOSAICUM, FX_DITHERING_FS, FX_SOLARIZATIO,
                FX_AURUM, FX_DITHERING_BAYER, FX_POSTERIZATIO
            };
            int n_bg_audax = (int)(sizeof(fx_bg_audax) / sizeof(fx_bg_audax[0]));
            static const PostEffectus fx_bg_moderatus[] = {
                FX_SCANLINEAE_CRT, FX_FRESCO, FX_PATINA, FX_RIMAE,
                FX_LINEAE_PROMINENTES, FX_NITOR, FX_ABER_CHROMATIS, FX_GRANUM
            };
            int n_bg_mod = (int)(sizeof(fx_bg_moderatus) / sizeof(fx_bg_moderatus[0]));

            /* Vis: trimodalis — subtilis/moderatus/maximus — ut spread late vistus sit */
            float vfg_r = sors_f32(&s);
            float vbg_r = sors_f32(&s);
            (void)0;
            #define SAMPLE_VIS_FG(SS)                               \
                ((vfg_r = sors_f32(SS)) < 0.30f                     \
                    ? sors_spatium(SS, 0.15f, 0.35f)                \
                    : vfg_r < 0.70f                                 \
                        ? sors_spatium(SS, 0.40f, 0.65f)            \
                        : sors_spatium(SS, 0.70f, 1.0f))
            #define SAMPLE_VIS_BG(SS)                               \
                ((vbg_r = sors_f32(SS)) < 0.25f                     \
                    ? sors_spatium(SS, 0.20f, 0.45f)                \
                    : vbg_r < 0.55f                                 \
                        ? sors_spatium(SS, 0.50f, 0.75f)            \
                        : sors_spatium(SS, 0.80f, 1.0f))

            PostEffectus fx_fg = FX_NULLUS, fx_bg = FX_NULLUS;
            float vis_fg = 0.0f, vis_bg = 0.0f;

            /* Scenaria: distributio mixta ut layering et unified looks servētur */
            float scen = sors_f32(&s);
            if (scen < 0.10f) {
                /* 10% nihil — mundus */
            } else if (scen < 0.45f) {
                /* 35% idem fx ambobus — classicus unified look */
                PostEffectus f = fx_fg_set[sors_proximus(&s) % (unsigned)n_fg];
                fx_fg = f;
                fx_bg = f;
                vis_fg = SAMPLE_VIS_FG(&s);
                vis_bg = saturatef(vis_fg + sors_spatium(&s, 0.0f, 0.25f));
            } else if (scen < 0.70f) {
                /* 25% diversi fx — different bg/fg per novum capability */
                fx_fg  = fx_fg_set[sors_proximus(&s) % (unsigned)n_fg];
                vis_fg = SAMPLE_VIS_FG(&s);
                int audax = sors_f32(&s) < 0.6f;
                fx_bg = audax
                    ? fx_bg_audax[sors_proximus(&s) % (unsigned)n_bg_audax]
                    : fx_bg_moderatus[sors_proximus(&s) % (unsigned)n_bg_mod];
                vis_bg = SAMPLE_VIS_BG(&s);
            } else if (scen < 0.88f) {
                /* 18% bg audax solus — vultus mundus, fundus dramaticus */
                fx_bg  = fx_bg_audax[sors_proximus(&s) % (unsigned)n_bg_audax];
                vis_bg = SAMPLE_VIS_BG(&s);
            } else {
                /* 12% fg solus */
                fx_fg  = fx_fg_set[sors_proximus(&s) % (unsigned)n_fg];
                vis_fg = SAMPLE_VIS_FG(&s);
            }
            #undef SAMPLE_VIS_FG
            #undef SAMPLE_VIS_BG

            /* Expressio: ~90% habet expressionem, ponderata favet extrema visibiliter */
            int with_expr = sors_f32(&s) < 0.90f;
            /* Pondera idx: 0=NEUTRUM 1=LAETUM 2=TRISTE 3=IRATUM 4=MIRANS 5=TIMIDUM 6=FASTIDIOSUM 7=CONTEMPTUM
             * Favent expressiōnes quae legibilēs sunt in tesserīs parvīs */
            float pond_expr[8] = {
                0.04f, 0.26f, 0.16f, 0.22f, 0.20f, 0.05f, 0.04f, 0.03f
            };
            int expr_idx = sors_ponderatus(&s, pond_expr, 8);
            Expressio expr = expr_tab[expr_idx];
            float pondus = with_expr ? sors_spatium(&s, 0.80f, 1.0f) : 0.0f;

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

            imago_redde_bgfg(im, cbuf, modus, fx_bg, vis_bg, fx_fg, vis_fg, tempus);
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

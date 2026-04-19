/* main.c — CLI pro portrait, cum switches et defaultis.
 *
 * Exemplum:
 *   ./portrait -o out.ppm
 *   ./portrait --semen 42 --archetypum lar --gens nympha --modus orientalis -o nympha.ppm
 *   ./portrait -s foo -a faunus -g furia -m comicus -t 0.3 -f vignetta -v 0.6 \
 *              -e laetum -p 1.0 -o furia.ppm
 */

#include "homuncio.h"
#include "sors.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usus(const char* argv0) {
    fprintf(
        stderr,
        "usus: %s [optiones]\n"
        "\n"
        "Optiones (omnes cum defaultis — argumentis nullis imago usitata scribitur):\n"
        "  -o, --output <via>       via exitus PPM                 [default: portrait.ppm]\n"
        "  -s, --semen <valor>      numerus vel nomen (hashed)     [default: 1]\n"
        "  -a, --archetypum <nomen> archetypum personae            [default: lar]\n"
        "  -g, --gens <nomen>       gens (species)                 [default: humana]\n"
        "  -m, --modus <nomen>      modus artis (stilus)           [default: comicus]\n"
        "  -t, --tempus <float>     [0,1), animatio periodica      [default: 0.0]\n"
        "  -f, --fx <nomen>         post-effectus                  [default: nullus]\n"
        "  -v, --vis-fx <float>     vis post-effectus [0,1]        [default: 0.5]\n"
        "  -e, --expressio <nomen>  vultus                         [default: neutrum]\n"
        "  -p, --pondus <float>     pondus expressionis [0,1]      [default: 0.0]\n"
        "  -h, --help               ostende hunc textum\n"
        "\n"
        "Valores possibiles:\n"
        "  archetypum:  lar manes lemur ianus augur quirinus carna genius flamen faunus\n"
        "  gens:        humana nympha pygmaeus gigas penates larva furia satyrus\n"
        "  modus:       comicus tessellatus atramentum pictum ludicrum_viii orientalis niger\n"
        "  fx:          nullus vignetta granum scanlineae_crt aber_chromatis\n"
        "               dithering_bayer dithering_fs halftone posterizatio\n"
        "               lineae_prominentes nitor patina fresco aurum mosaicum\n"
        "               solarizatio rimae\n"
        "  expressio:   neutrum laetum triste iratum mirans timidum fastidiosum contemptum\n"
        "\n"
        "Semen: integer (e.g. 42) vel nomen arbitrarium (e.g. 'marcus') — hashed deterministice.\n",
        argv0
    );
}

static Expressio expressio_ex_nomine(const char* n) {
    if (strcmp(n, "laetum") == 0)
        return EXPR_LAETUM;
    if (strcmp(n, "triste") == 0)
        return EXPR_TRISTE;
    if (strcmp(n, "iratum") == 0)
        return EXPR_IRATUM;
    if (strcmp(n, "mirans") == 0)
        return EXPR_MIRANS;
    if (strcmp(n, "timidum") == 0)
        return EXPR_TIMIDUM;
    if (strcmp(n, "fastidiosum") == 0)
        return EXPR_FASTIDIOSUM;
    if (strcmp(n, "contemptum") == 0)
        return EXPR_CONTEMPTUM;
    if (strcmp(n, "neutrum") == 0)
        return EXPR_NEUTRUM;
    return EXPR_NEUTRUM;
}

/* Parse option argument: supports "-f val", "--foo val", and "--foo=val" forms.
 * Returns the value pointer or NULL, and advances *pi if separate arg consumed.
 * arg_equals is the "=value" offset within argv[i] if present, else NULL.
 */
static const char* consume_val(int argc, char** argv, int* pi, const char* arg_equals) {
    if (arg_equals && *arg_equals)
        return arg_equals;
    if (*pi + 1 >= argc)
        return NULL;
    (*pi)++;
    return argv[*pi];
}

/* Matches "-X" or "--name" (and "--name=..." via eq). Returns "=tail" or "" or NULL. */
static const char* match_flag(const char* arg, char shortc, const char* longname) {
    /* short form: exactly "-X" (with possible "=val"? standard is no, but allow "-X=val") */
    if (arg[0] == '-' && arg[1] == shortc && (arg[2] == '\0' || arg[2] == '=')) {
        return arg[2] == '=' ? arg + 3 : "";
    }
    /* long form */
    size_t ln = strlen(longname);
    if (arg[0] == '-' && arg[1] == '-' && strncmp(arg + 2, longname, ln) == 0) {
        char after = arg[2 + ln];
        if (after == '\0')
            return "";
        if (after == '=')
            return arg + 2 + ln + 1;
    }
    return NULL;
}

int main(int argc, char** argv) {
    /* Defaultis */
    const char* via          = "portrait.ppm";
    uint64_t    semen        = 1;
    int         arch         = ARCH_LAR;
    int         gens         = GENS_HUMANA;
    int         modus        = MODUS_COMICUS;
    float       tempus       = 0.0f;
    PostEffectus fx          = FX_NULLUS;
    float       vis_fx       = 0.5f;
    Expressio   expr         = EXPR_NEUTRUM;
    float       pondus       = 0.0f;

    for (int i = 1; i < argc; i++) {
        const char* a = argv[i];
        const char* eq;

        if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
            usus(argv[0]);
            return 0;
        }

        if ((eq = match_flag(a, 'o', "output")) != NULL) {
            const char* v = consume_val(argc, argv, &i, eq);
            if (!v) {
                fprintf(stderr, "optio --output requirit valorem\n");
                return 1;
            }
            via = v;
            continue;
        }
        if ((eq = match_flag(a, 's', "semen")) != NULL) {
            const char* v = consume_val(argc, argv, &i, eq);
            if (!v) {
                fprintf(stderr, "optio --semen requirit valorem\n");
                return 1;
            }
            if (v[0] >= '0' && v[0] <= '9')
                semen = (uint64_t) strtoull(v, NULL, 10);
            else
                semen = sors_hash_nominis(v);
            continue;
        }
        if ((eq = match_flag(a, 'a', "archetypum")) != NULL) {
            const char* v = consume_val(argc, argv, &i, eq);
            if (!v) {
                fprintf(stderr, "optio --archetypum requirit valorem\n");
                return 1;
            }
            arch = portrait_archetypum_ex_nomine(v);
            if (arch < 0) {
                fprintf(stderr, "archetypum ignotum: %s\n", v);
                return 1;
            }
            continue;
        }
        if ((eq = match_flag(a, 'g', "gens")) != NULL) {
            const char* v = consume_val(argc, argv, &i, eq);
            if (!v) {
                fprintf(stderr, "optio --gens requirit valorem\n");
                return 1;
            }
            gens = portrait_gens_ex_nomine(v);
            if (gens < 0) {
                fprintf(stderr, "gens ignota: %s\n", v);
                return 1;
            }
            continue;
        }
        if ((eq = match_flag(a, 'm', "modus")) != NULL) {
            const char* v = consume_val(argc, argv, &i, eq);
            if (!v) {
                fprintf(stderr, "optio --modus requirit valorem\n");
                return 1;
            }
            modus = portrait_modus_ex_nomine(v);
            if (modus < 0) {
                fprintf(stderr, "modus ignotus: %s\n", v);
                return 1;
            }
            continue;
        }
        if ((eq = match_flag(a, 't', "tempus")) != NULL) {
            const char* v = consume_val(argc, argv, &i, eq);
            if (!v) {
                fprintf(stderr, "optio --tempus requirit valorem\n");
                return 1;
            }
            tempus = (float) atof(v);
            continue;
        }
        if ((eq = match_flag(a, 'f', "fx")) != NULL) {
            const char* v = consume_val(argc, argv, &i, eq);
            if (!v) {
                fprintf(stderr, "optio --fx requirit valorem\n");
                return 1;
            }
            int e = portrait_effectus_ex_nomine(v);
            if (e < 0) {
                fprintf(stderr, "fx ignotus: %s\n", v);
                return 1;
            }
            fx = (PostEffectus) e;
            continue;
        }
        if ((eq = match_flag(a, 'v', "vis-fx")) != NULL) {
            const char* v = consume_val(argc, argv, &i, eq);
            if (!v) {
                fprintf(stderr, "optio --vis-fx requirit valorem\n");
                return 1;
            }
            vis_fx = (float) atof(v);
            continue;
        }
        if ((eq = match_flag(a, 'e', "expressio")) != NULL) {
            const char* v = consume_val(argc, argv, &i, eq);
            if (!v) {
                fprintf(stderr, "optio --expressio requirit valorem\n");
                return 1;
            }
            expr = expressio_ex_nomine(v);
            /* si pondus nondum datum, defaulta ad 1.0 quando expressio explicita */
            if (pondus < 0.001f)
                pondus = 1.0f;
            continue;
        }
        if ((eq = match_flag(a, 'p', "pondus")) != NULL) {
            const char* v = consume_val(argc, argv, &i, eq);
            if (!v) {
                fprintf(stderr, "optio --pondus requirit valorem\n");
                return 1;
            }
            pondus = (float) atof(v);
            continue;
        }

        fprintf(stderr, "argumentum ignotum: %s\n", a);
        fprintf(stderr, "proba '%s --help' pro auxilio.\n", argv[0]);
        return 1;
    }

    /* modulus periodicus: [0, 1) */
    tempus = tempus - (float) floor((double) tempus);

    Imago* im = imago_nova(semen, (Archetypum) arch, (Gens) gens);
    if (!im) {
        fprintf(stderr, "allocatio frustra\n");
        return 1;
    }
    if (pondus > 0.001f) {
        imago_pone_expressionem(im, expr, pondus);
    }

    uint8_t* buf = (uint8_t*) malloc((size_t) PORTRAIT_LATITUDO * PORTRAIT_ALTITUDO * 4u);
    if (!buf) {
        imago_dele(im);
        return 1;
    }

    imago_redde(im, buf, (ModusArtis) modus, fx, vis_fx, tempus);

    extern int exitus_scribe_ppm(const char*, const uint8_t*, int, int);
    int ret = exitus_scribe_ppm(via, buf, PORTRAIT_LATITUDO, PORTRAIT_ALTITUDO);
    free(buf);
    imago_dele(im);

    if (ret != 0) {
        fprintf(stderr, "scriptio PPM frustra: %s\n", via);
        return 1;
    }
    return 0;
}

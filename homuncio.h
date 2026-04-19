/* homuncio.h — caput publicum bibliothecae portrait.
 *
 * Generator imaginum personarum in C99 purus, nullas dependentias externas habens.
 * Omnis pixel ex uint64_t semine mathematice derivatur; nullae imagines externae
 * adhibentur. Exitus: PPM binarius (P6), 128×128 puncta.
 *
 * Usus brevissimus:
 *   imago_crea_ppm(seed, ARCH_LAR, GENS_HUMANA, MODUS_COMICUS, 0.0f, "out.ppm");
 */
#ifndef HOMUNCIO_H
#define HOMUNCIO_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Enumerationes publicae
 * ------------------------------------------------------------------------ */

    typedef enum {
        FORMA_OVALIS = 0,
        FORMA_ROTUNDA,
        FORMA_QUADRATA,
        FORMA_CORDIS,
        FORMA_RHOMBI,
        FORMA_OBLONGA,
        FORMA_TRIANGULARIS,
        FORMA_RECTA,
        FORMA_NUMERUS,
    } FormaFaciei;

/* Archetypa ex religione domestica Romana — spiritus, genii, sacerdotes.
 * Unumquodque archetypum biasat parametra generationis ad personam signatam. */
    typedef enum {
        ARCH_LAR = 0,      /* custos domus — forma iuvenilis, cutis calida, oculi clari */
        ARCH_MANES,        /* maiorum spiritus divini — aetate graves, rugae profundae, cani */
        ARCH_LEMUR,        /* mortuus inquietus malevolus — macilentus, toni obscuri, palpebrae graves */
        ARCH_IANUS,        /* deus bifrons liminum — asymmetricus, callidus, supercilia expressa */
        ARCH_AUGUR,        /* sacerdos ominum interpres — nasus prominens, pallidus, sollicitus */
        ARCH_QUIRINUS,     /* Romulus divus, deus militaris — cicatrices, cutis intemperie tacta */
        ARCH_CARNA,        /* dea valetudinis et viscerum — lenia lineamenta, vultus benignus */
        ARCH_GENIUS,       /* spiritus personalis divinus — iuvenis, rotundus, oculi magni */
        ARCH_FLAMEN,       /* sacerdos uni numini dedicatus — elegantia, compositus, malae altae */
        ARCH_FAUNUS,       /* spiritus silvester versipellis — asymmetricus, subrisus, scientes oculi */
        ARCH_NUMERUS,
    } Archetypum;

/* Gentes ex mythologia Romana — non ex traditione fantastica moderna. */
    typedef enum {
        GENS_HUMANA = 0,      /* mortales */
        GENS_NYMPHARUM,       /* nymphae — spiritus pulchri naturae, alti, aures acutae */
        GENS_PYGMAEORUM,      /* pygmaei — gens parva bellicosa apud Plinium et Homerum, gruibus hostis */
        GENS_GIGANTUM,        /* gigantes — immanes, dentes exserti, cutis tincta */
        GENS_PENATIUM,        /* penates — parvi rotundi, protectores penuariae */
        GENS_LARVARUM,        /* larvae — umbrae mortuorum, pallidae, oculi concavi */
        GENS_FURIARUM,        /* furiae — spiritus vindictae, oculi sanguinei, pupillae fissae, serpentes in coma */
        GENS_SATYRORUM,       /* satyri — rustici silvestres, cornicula parva, nasus magnus */
        GENS_NUMERUS,
    } Gens;

    typedef enum {
        COMA_BREVIS_RECTA = 0,
        COMA_MEDIA_UNDATA,
        COMA_LONGA_RECTA,
        COMA_CRISPATA,
        COMA_GLOBOSA,
        COMA_CALVA,
        COMA_CRISTA,
        COMA_NODUS,
        COMA_EFFUSA,
        COMA_NUMERUS,
    } ModusComae;

    typedef enum {
        BARBA_NULLA = 0,
        BARBA_STIRPS,      /* tantum stirps (stubble) */
        BARBA_MAXILLAE,    /* circum maxillam */
        BARBA_PLENA,
        BARBA_PROLIXA,
        BARBA_BIFIDA,
        BARBA_NUMERUS,
    } ModusBarbae;

    typedef enum {
        DENTES_NULLI = 0,        /* nulli conspicui */
        DENTES_CANINI_INFERI,    /* duo dentes acuti ex labio inferiore — gigantes, orci */
        DENTES_CANINI_SUPERI,    /* duo dentes acuti descendentes super labium inferius */
        DENTES_SINGULARIS,       /* unus tantum dens — asymmetricus, comicus */
        DENTES_RUPTI,            /* ordo interruptus cum lacunis — aetas aut pugna */
        DENTES_ORDO,             /* ordo plenus albus — subrisus patens */
        DENTES_NUMERUS,
    } ModusDentium;

    typedef enum {
        MUSTACIA_NULLA = 0,
        MUSTACIA_SIMPLEX,
        MUSTACIA_LATA,
        MUSTACIA_IMPERATORIA,
        MUSTACIA_NUMERUS,
    } ModusMustaciorum;

/* Vestimenta Romana — super umeros pingitur */
    typedef enum {
        VESTIS_NULLA = 0,     /* nudus */
        VESTIS_TUNICA,        /* simplex cum rotundo collo */
        VESTIS_TOGA,          /* cum plicis drapeis et clavo */
        VESTIS_STOLA,         /* muliebris cum fibula */
        VESTIS_LORICA,        /* armatura segmentata militaris */
        VESTIS_PALLIUM,       /* pallium cum claspe asymmetrico */
        VESTIS_NUMERUS,
    } ModusVestis;

/* Ornamenta capitis — super comam pingitur */
    typedef enum {
        ORNAMENTUM_NULLUM = 0,
        ORNAMENTUM_CORONA_LAUREA,   /* corona foliorum lauri */
        ORNAMENTUM_GALEA,           /* galea militaris cum crista */
        ORNAMENTUM_VITTA,           /* fascia sacerdotalis */
        ORNAMENTUM_PILEUS,          /* pileus libertatis conicus */
        ORNAMENTUM_DIADEMA,         /* diadema aureum cum gemma */
        ORNAMENTUM_FEX,             /* pileum cylindricum truncatum cum fimbria pendente */
        ORNAMENTUM_NUMERUS,
    } ModusOrnamenti;

    typedef enum {
        MODUS_COMICUS = 0,       /* rīsum movēns, trēs tonī + līneāmenta */
        MODUS_TESSELLATUS,       /* more mūsīvī Rōmānī — tēsserae quadrātae */
        MODUS_ATRAMENTUM,
        MODUS_PICTUM,
        MODUS_LUDICRUM_VIII,
        MODUS_ORIENTALIS,        /* stylus Orientis — colōrēs saturātī, cellae nitidae */
        MODUS_NIGER,
        MODUS_NUMERUS,
    } ModusArtis;

    typedef enum {
        FX_NULLUS = 0,
        FX_VIGNETTA,
        FX_GRANUM,
        FX_SCANLINEAE_CRT,
        FX_ABER_CHROMATIS,
        FX_DITHERING_BAYER,
        FX_DITHERING_FS,
        FX_HALFTONE,
        FX_POSTERIZATIO,
        FX_LINEAE_PROMINENTES,
        FX_NITOR,
        FX_PATINA,              /* oxidatio aerea — viridis-caeruleus in umbris */
        FX_FRESCO,              /* stylus Pompeianus — tonus terreus + mollitia */
        FX_AURUM,               /* folium aureum super lucidissimas partes */
        FX_MOSAICUM,            /* tesserae cum lineis commissuralibus */
        FX_SOLARIZATIO,         /* inversio tonalis super limine */
        FX_RIMAE,               /* rete rimarum mirariarum */
        FX_NUMERUS,
    } PostEffectus;

    typedef enum {
        MARGO_NULLUS = 0,
        MARGO_LAPIDIS,
        MARGO_LIGNI,
        MARGO_ARCANUM,
        MARGO_FERRI,
        MARGO_NUMERUS,
    } MargoModus;

/* ---------------------------------------------------------------------------
 * Expressio — deltae parametrorum pro vultibus
 * ------------------------------------------------------------------------ */

    typedef struct {
        float supercilium_leva_int, supercilium_dex_int;
        float supercilium_leva_ext, supercilium_dex_ext;
        float contractio_supercilii;
        float apertio_oculi_sin, apertio_oculi_dex;
        float contractio_oculi_sin, contractio_oculi_dex;
        float apertio_oris;
        float risus;
        float latitudo_oris;
        float tractio_anguli_sin, tractio_anguli_dex;
        float dilatatio_narium;
        float elevatio_genae;
        float elevatio_menti;
        float descensus_maxillae;
    } Expressio;

    extern const Expressio EXPR_NEUTRUM;
    extern const Expressio EXPR_LAETUM;
    extern const Expressio EXPR_TRISTE;
    extern const Expressio EXPR_IRATUM;
    extern const Expressio EXPR_MIRANS;
    extern const Expressio EXPR_TIMIDUM;
    extern const Expressio EXPR_FASTIDIOSUM;
    extern const Expressio EXPR_CONTEMPTUM;

    Expressio expressio_misce(Expressio a, Expressio b, float t);

/* ---------------------------------------------------------------------------
 * Imago — typus opacus
 * ------------------------------------------------------------------------ */

    typedef struct Imago Imago;

    Imago* imago_nova(uint64_t semen, Archetypum arch, Gens gens);
    void   imago_dele(Imago* im);

/* Mutatio per nomen clavis (mutat solum parametra quae nomine notiuntur;
 * parametra ignota silenter praetereuntur). */
    void imago_pone_parametrum(Imago* im, const char* clavis, float valor);

/* Expressionem applicat (pondus 0..1 miscet cum neutro) */
    void imago_pone_expressionem(Imago* im, Expressio expr, float pondus);

/* Redditio ad capsam RGBA de 128×128 (4 octeti per pixel) */
    void imago_redde(
        Imago* im, uint8_t* rgba, ModusArtis modus,
        PostEffectus fx, float vis_fx, float tempus
    );

/* Sicut imago_redde, sed duo post-effectus in sequentia applicat.
 * FX_NULLUS in fx2 reducit ad imago_redde normalem. */
    void imago_redde_fx2(
        Imago* im, uint8_t* rgba, ModusArtis modus,
        PostEffectus fx1, float vis_fx1,
        PostEffectus fx2, float vis_fx2,
        float tempus
    );

/* Redditio in duobus stratis: fx separati applicantur fundo et figurae.
 * Permittit fx audaces in fundo (e.g. halftone, posterizatio) sine corruptione
 * figurae.  fx_bg/fx_fg potest esse FX_NULLUS pro nullo effectu. */
    void imago_redde_bgfg(
        Imago* im, uint8_t* rgba, ModusArtis modus,
        PostEffectus fx_bg, float vis_bg,
        PostEffectus fx_fg, float vis_fg,
        float tempus
    );

/* Commoditas: creat, reddit, scribit PPM, deletque. Redit 0 si sucessus. */
    int imago_crea_ppm(
        uint64_t semen, Archetypum arch, Gens gens,
        ModusArtis modus, float tempus, const char* via
    );

/* Dialogi imago: imago + margo decorativus circum, 128×128 */
    void imago_margo_dialogi(
        Imago* im, uint8_t* rgba, ModusArtis modus,
        MargoModus margo, float tempus
    );

/* --- Auxilia textuales pro CLI: conversio nomen -> enum, redit -1 si ignotus --- */
    int portrait_archetypum_ex_nomine(const char* nomen);
    int portrait_gens_ex_nomine(const char* nomen);
    int portrait_modus_ex_nomine(const char* nomen);
    int portrait_effectus_ex_nomine(const char* nomen);

/* Magnitudo imaginis nativae */
#define PORTRAIT_LATITUDO 128
#define PORTRAIT_ALTITUDO 128

#ifdef __cplusplus
}
#endif
#endif /* HOMUNCIO_H */

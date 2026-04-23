#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

/* xorshift64* PRNG — deterministic from seed */
static uint64_t rng_state;

static void rng_seed(uint64_t s) {
    if (s == 0) s = 0x9E3779B97F4A7C15ULL;
    rng_state = s;
}

static uint64_t rng_next(void) {
    uint64_t x = rng_state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    rng_state = x;
    return x * 0x2545F4914F6CDD1DULL;
}

static uint32_t rng_range(uint32_t n) { return (uint32_t)(rng_next() % n); }
static double rng_unit(void) { return (rng_next() >> 11) * (1.0 / 9007199254740992.0); }
static int rng_chance(double p) { return rng_unit() < p; }

/* weighted pick: weights sum need not be 1 */
static int rng_weighted(const double *w, int n) {
    double total = 0;
    for (int i = 0; i < n; i++) total += w[i];
    double r = rng_unit() * total;
    double acc = 0;
    for (int i = 0; i < n; i++) { acc += w[i]; if (r < acc) return i; }
    return n - 1;
}

static const char *pick(const char *const *arr, int n) { return arr[rng_range(n)]; }

static const char *article(const char *s) {
    char c = s[0];
    if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') ? "an" : "a";
}

/* ---- tables ---- */

/* hair colors tagged by compatible skin: 'L'=light, 'M'=medium, 'D'=dark; string lists all allowed */
typedef struct { const char *text; const char *skins; } HairColor;
static const HairColor hair_colors_young[] = {
    {"jet black","LMD"},{"pitch black","LMD"},{"raven black","LMD"},
    {"coal black","LMD"},{"inky black","LMD"},
    {"dark brown","LMD"},{"deep brown","LMD"},{"espresso brown","LMD"},
    {"chocolate brown","LMD"},{"chestnut brown","LM"},
    {"walnut brown","LM"},{"medium brown","LM"},{"mousy brown","LM"},
    {"ash brown","LM"},{"light brown","LM"},
    {"sandy brown","L"},{"mahogany brown","LM"},
    {"dirty blonde","L"},{"dark blonde","L"},{"honey blonde","L"},
    {"golden blonde","L"},{"wheat blonde","L"},{"sun-bleached blonde","L"},
    {"platinum blonde","L"},{"flaxen blonde","L"},{"pale blonde","L"},
    {"strawberry blonde","L"},
    {"auburn","LM"},{"dark auburn","LM"},
    {"copper red","L"},{"coppery red","L"},{"ginger","L"},
    {"fiery red","L"},{"burnt orange","L"},{"rust red","L"},{"titian red","L"}
};
static const char *const hair_colors_graying[] = {
    "salt-and-pepper","pepper-and-salt","mostly gray with dark streaks",
    "dark with scattered gray","graying at the temples","silver-gray","pewter gray",
    "ash brown flecked with gray","pepper gray","steel-streaked","half-silvered"
};
static const char *const hair_colors_old[] = {
    "pure white","snow white","chalk white","silver","silvery","iron gray",
    "slate gray","pale gray","thinning gray","wispy white","bone white"
};

/* feature typed by sex ('u' unisex, 'm', 'f') and quality ('u' ugly/plain, 'n' neutral, 'p' pretty) */
typedef struct { const char *text; char sex; char qual; } Feat;

static const Feat eye_colors[] = {
    {"deep brown",'u','n'},{"dark brown",'u','n'},{"coffee brown",'u','n'},
    {"chocolate brown",'u','n'},{"near-black",'u','n'},{"walnut brown",'u','n'},
    {"rich brown",'u','n'},{"warm brown",'u','n'},{"russet brown",'u','n'},
    {"tawny brown",'u','n'},
    {"warm amber",'u','n'},{"golden amber",'u','n'},{"honey-colored",'u','n'},
    {"honey amber",'u','n'},{"topaz",'u','n'},
    {"hazel",'u','n'},{"golden hazel",'u','n'},{"green-flecked hazel",'u','n'},
    {"light hazel",'u','n'},
    {"olive green",'u','n'},{"moss green",'u','n'},{"forest green",'u','n'},
    {"sea green",'u','n'},{"emerald green",'u','n'},{"jade green",'u','n'},
    {"mossy green",'u','n'},{"pale green",'u','n'},{"celadon green",'u','n'},
    {"gray-green",'u','n'},
    {"slate blue",'u','n'},{"steel blue",'u','n'},{"pale blue",'u','n'},
    {"icy blue",'u','n'},{"cornflower blue",'u','n'},{"deep blue",'u','n'},
    {"sky blue",'u','n'},{"navy blue",'u','n'},{"sapphire blue",'u','n'},
    {"robin's-egg blue",'u','n'},{"turquoise",'u','n'},{"aquamarine",'u','n'},
    {"blue-gray",'u','n'},
    {"stormy gray",'u','n'},{"steel gray",'u','n'},{"pale gray",'u','n'},
    {"silver-gray",'u','n'},{"slate gray",'u','n'},{"dove gray",'u','n'},
    {"smoky gray",'u','n'},
    {"green-flecked brown",'u','n'},{"amber-flecked brown",'u','n'},
    {"violet-blue",'u','p'},
    {"heterochromatic (one blue, one brown)",'u','n'},
    {"small and dull",'u','u'},{"small and deep-set",'u','u'},
    {"watery pale",'u','u'},{"bloodshot brown",'u','u'},
    {"dull gray",'u','u'},{"close-set and dark",'u','u'},
    {"deep-set and shadowed",'u','u'},{"tired-looking brown",'u','u'},
    {"piercing and pale",'u','p'},{"warm and brown",'u','n'},
    {"wide-set brown",'u','n'},{"narrow almond-shaped brown",'u','n'},
    {"almond-shaped black",'u','n'},
    {"large and expressive dark",'u','p'},{"luminous brown",'u','p'},
    {"bright green",'u','p'},{"sparkling blue",'u','p'},
    {"captivating hazel",'u','p'},
    {"thickly lashed brown",'f','p'},{"thickly lashed green",'f','p'},
    {"bright and clear blue",'u','p'},{"strikingly pale blue",'u','p'},
    {"jewel-like emerald",'u','p'},{"wide and expressive",'u','p'},
    {"doe-like brown",'f','p'},{"almond-shaped green",'u','p'},
    {"almond-shaped hazel",'u','p'},{"elegantly almond-shaped dark",'u','p'},
    {"strikingly bright",'u','p'},{"luminous pale gray",'u','p'}
};

/* skin tones tagged by lightness category */
typedef struct { const char *text; char cat; } SkinTone;
static const SkinTone skin_tones[] = {
    {"very pale",'L'},{"porcelain pale",'L'},{"alabaster",'L'},
    {"ghostly pale",'L'},{"fair",'L'},{"light",'L'},{"light with freckles",'L'},
    {"cream-pale",'L'},{"ivory",'L'},{"light olive",'L'},
    {"warm olive",'M'},{"sallow olive",'M'},{"sun-warmed",'M'},
    {"golden tan",'M'},{"tan",'M'},{"sun-tanned",'M'},{"deep tan",'M'},
    {"bronzed",'M'},{"warm beige",'M'},{"medium brown",'M'},{"warm brown",'M'},
    {"rich brown",'D'},{"russet",'D'},{"umber",'D'},
    {"dark brown",'D'},{"deep brown",'D'},{"mahogany",'D'},
    {"ebony",'D'},{"deep ebony",'D'},{"near-black",'D'}
};

static const Feat face_shapes[] = {
    {"oval",'u','n'},{"long oval",'u','n'},{"narrow oval",'u','n'},
    {"egg-shaped",'u','n'},{"round",'u','n'},{"full-round",'u','n'},
    {"full-cheeked",'u','n'},{"moon-faced",'u','u'},
    {"cherubic",'f','n'},{"square-jawed",'u','n'},{"square-cut",'u','n'},
    {"broad-jawed",'m','n'},{"strong-jawed",'m','n'},
    {"blocky and square",'m','u'},{"heart-shaped",'u','n'},
    {"inverted-triangle",'u','n'},{"tapering",'u','n'},
    {"long and angular",'u','n'},{"lean and angular",'u','n'},
    {"sharply angular",'u','n'},{"long and narrow",'u','n'},
    {"narrow and long",'u','n'},{"broad and flat-cheeked",'u','n'},
    {"wide and high-cheeked",'u','n'},{"high-cheekboned",'u','p'},
    {"prominent-cheekboned",'u','p'},{"narrow and pointed-chinned",'u','n'},
    {"narrow and tapered",'u','n'},{"diamond-shaped",'u','n'},
    {"gaunt and hollow-cheeked",'u','u'},{"sunken-cheeked",'u','u'},
    {"soft and rounded",'u','n'},{"softly rounded",'u','n'},
    {"plump and round",'u','u'},{"chiseled",'u','p'},
    {"sculpted",'u','p'},{"sharply chiseled",'m','p'},
    {"oblong",'u','n'},{"pear-shaped",'u','u'},
    {"wide-browed and narrow-chinned",'u','n'},{"heavy-browed",'m','u'},
    {"heavy-jawed",'m','u'},{"boyish and round",'u','n'},
    {"delicate and oval",'f','p'},{"fine-boned",'u','p'},
    {"broad-browed",'u','n'},{"lopsided and asymmetrical",'u','u'},
    {"coarse-featured",'u','u'},{"blotchy and uneven",'u','u'},
    {"pudgy and round",'u','u'},{"pock-marked",'u','u'},
    {"strikingly symmetrical",'u','p'},{"finely sculpted",'u','p'},
    {"elegantly shaped",'u','p'},{"graceful",'f','p'},
    {"beautifully proportioned",'u','p'},{"classically sculpted",'u','p'},
    {"model-like",'u','p'},{"high-cheekboned and elegant",'u','p'},
    {"sharply defined and balanced",'u','p'},{"softly sculpted",'u','p'},
    {"porcelain-smooth and oval",'f','p'},{"swan-necked and oval",'f','p'}
};

static const Feat nose_shapes[] = {
    {"straight",'u','n'},{"straight and even",'u','n'},
    {"straight and narrow",'u','n'},{"perfectly straight",'u','p'},
    {"slightly aquiline",'u','n'},{"aquiline",'u','n'},
    {"strongly aquiline",'u','n'},{"hooked",'u','u'},
    {"sharply hooked",'u','u'},{"eagle-like",'u','u'},
    {"roman",'u','n'},{"small and upturned",'u','n'},
    {"small and buttoned",'f','n'},{"button-like",'f','n'},
    {"pert",'u','n'},{"pert and upturned",'u','n'},
    {"slightly upturned",'u','n'},{"broad",'u','n'},{"wide",'u','n'},
    {"flat and broad",'u','u'},{"broad-based",'u','n'},
    {"wide-nostriled",'u','u'},{"narrow",'u','n'},
    {"thin and narrow",'u','n'},{"slim",'u','n'},
    {"pinched",'u','u'},{"slightly bumped",'u','u'},
    {"hump-bridged",'u','u'},{"refined and tapered",'u','p'},
    {"refined",'u','p'},{"delicate",'f','p'},{"delicately shaped",'f','p'},
    {"long and slender",'u','n'},{"long and narrow",'u','n'},
    {"long and thin",'u','n'},{"short and snub",'u','n'},
    {"snub",'u','n'},{"short and broad",'u','u'},
    {"bulbous-tipped",'u','u'},{"round-tipped",'u','u'},
    {"ski-jump",'u','n'},{"slightly crooked",'u','u'},
    {"off-center",'u','u'},{"prominent",'u','n'},
    {"prominent-bridged",'u','n'},{"flattened",'u','u'},
    {"flared-nostriled",'u','u'},{"Greek-straight",'u','n'},
    {"gently curved",'u','n'},{"drooping-tipped",'u','u'},
    {"elegantly shaped",'u','p'},{"perfectly proportioned",'u','p'},
    {"finely shaped",'u','p'},{"classically sculpted",'u','p'},
    {"beautifully straight",'u','p'},{"gracefully tapered",'u','p'},
    {"lumpy",'u','u'},{"crooked and scarred",'u','u'},
    {"heavy and thick",'u','u'},{"misshapen",'u','u'}
};

static const Feat lip_shapes[] = {
    {"full",'u','n'},{"plump",'f','n'},{"generously full",'f','n'},
    {"thin",'u','n'},{"narrow",'u','n'},{"thin and pursed",'u','u'},
    {"narrow upper and fuller lower",'u','n'},{"cupid's-bow",'f','p'},
    {"bow-shaped",'f','p'},{"wide",'u','n'},{"broad",'u','n'},
    {"small and delicate",'f','n'},{"small and tight",'u','u'},
    {"evenly shaped",'u','n'},{"even",'u','n'},
    {"soft and pillowy",'f','p'},{"firm and pressed",'m','n'},
    {"chapped and cracked",'u','u'},{"thin and mean-looking",'u','u'},
    {"too-thin",'u','u'},{"downturned",'u','u'},
    {"full and shapely",'f','p'},{"sensual",'f','p'},
    {"well-defined",'u','p'},{"beautifully shaped",'u','p'},
    {"perfectly shaped",'u','p'},{"softly curved",'u','p'},
    {"lush",'f','p'},{"elegantly bow-shaped",'f','p'},
    {"heart-shaped and full",'f','p'},{"sculpted and full",'u','p'}
};

static const char *const builds_thin[] = {
    "slight","slender","thin","spare","wiry","lanky","lean","willowy",
    "narrow","narrow-shouldered","reedy","gaunt","rail-thin","lean and narrow-shouldered"
};
static const char *const builds_avg[]  = {
    "average","of average build","medium-framed","athletic","trim","fit",
    "solidly built","sturdy","well-proportioned","compact","moderately built"
};
static const char *const builds_heavy[]= {
    "broad","broad-shouldered","heavyset","stocky","thickset","burly",
    "stout","portly","beefy","round-shouldered","barrel-chested",
    "broad and heavyset","big-boned","wide-framed"
};

/* hairstyles segmented by sex and length */
/* hairstyles written as noun phrases so they fit "worn in <X>" / "styled as <X>" */
static const char *const male_short[] = {
    "a close-cropped buzz cut","a shaved military cut","a tidy side-parted cut",
    "a neatly combed-back style","a slick side-part","a short textured crop",
    "a short messy cut","a clean fade","a skin fade","a high-and-tight cut",
    "a slightly tousled short cut","a short curly cut close to the scalp",
    "a short crew cut","a close-cropped style with shaved sides",
    "a short undercut with longer hair on top","a side-shaved cut with a swept-over top",
    "a short classic pompadour","a slicked-back short cut with a sharp part",
    "a short quiff swept up in front","a short faux-hawk",
    "a short wet-look slicked-back cut","a short Caesar cut with a straight fringe",
    "a short Ivy League cut","a short French crop with a textured fringe",
    "a short afro, close to the head","a short high-top fade",
    "short box braids close to the scalp","short twisted coils",
    "short dreadlocks","short cornrows running straight back",
    "a chopped punk cut with uneven lengths","a short mohawk laid flat over the scalp"
};
static const char *const male_medium[] = {
    "a medium-length style swept to one side","a shoulder-length wavy cut",
    "a shaggy medium cut falling over the ears","a curly medium-length style",
    "an ear-length wavy cut","a loose medium cut parted in the middle",
    "a thick medium cut combed back","a tousled medium-length wavy style",
    "a medium shag with layered ends","a medium mullet",
    "a medium undercut with long hair flowing on top",
    "a curtained medium cut with a center part",
    "an ear-length bob cut","a medium-length slicked-back style",
    "medium-length dreadlocks","medium-length twists",
    "a surfer's medium wavy cut"
};
static const char *const male_long[] = {
    "a long low ponytail","long loose waves past the shoulders",
    "long straight hair parted in the middle",
    "long unkempt hair past the shoulders","long curls to the collarbone",
    "long dreadlocks falling past the shoulders",
    "long braids gathered at the back","a long man-bun at the nape",
    "long layered hair with natural waves","long sleek hair parted to one side"
};
static const char *const female_short[] = {
    "a sharp pixie cut","a soft pixie cut","a chin-length bob","a blunt chin-length bob",
    "a cropped undercut","a short tousled cut","a sleek bob parted to one side",
    "a choppy short cut","a short cropped curly cut","a cropped shag cut",
    "an asymmetric bob, longer on one side","a chin-length bob with blunt bangs",
    "a jaw-length bob with curtain bangs","a short shag with micro-bangs",
    "a textured pixie with a long side fringe","a short afro, close to the head",
    "short box braids close to the scalp","short cornrows running straight back",
    "a faux-hawk with shaved sides","a short finger-waved cut",
    "a jaw-length blunt cut with a middle part","a chin-length bob with a deep side part",
    "a crop with a fringe cut straight across the brow"
};
static const char *const female_medium[] = {
    "a shoulder-length wavy cut","a straight medium cut with blunt ends",
    "loose medium-length curls","a layered shoulder-length style",
    "a collarbone-length cut with soft layers","a medium-length wavy style",
    "a medium-length low ponytail","a shoulder-length shag",
    "shoulder-length hair with curtain bangs","a shoulder-length lob with beachy waves",
    "a shoulder-length cut with blunt micro-bangs",
    "a crimped shoulder-length cut","a finger-waved medium cut",
    "medium-length hair in loose barrel curls",
    "medium-length hair with a middle part and flipped-out ends",
    "a shoulder-length cut with a deep side part","medium-length twists",
    "a medium-length hime cut with straight sidelocks"
};
static const char *const female_long[] = {
    "long straight hair past the shoulders","a long cascade down the back",
    "long loose waves","long flowing waves","long spiraling curls","long tight ringlets",
    "a thick braid draped over one shoulder","two loose braids",
    "a long fishtail braid draped forward","a long French braid laid down one shoulder",
    "long box braids","long dreadlocks falling past the shoulders",
    "a long hime cut with blunt sidelocks and long back",
    "long hair with blunt bangs","long hair with curtain bangs framing the face",
    "a long layered cut with face-framing pieces",
    "long mermaid waves","long hair with a crown of small braids",
    "long hair with soft bohemian waves","a long sleek cut with a sharp middle part"
};

static const char *const beards[] = {
    "a full thick beard","a full bushy beard","a short neatly trimmed beard",
    "a close-cropped beard","heavy stubble","a few days' stubble","light stubble",
    "a goatee","a neat goatee","a thin mustache","a thick mustache","a mustache only",
    "a long untamed beard streaked with gray","a long flowing beard","chin whiskers only",
    "a wiry graying beard","a carefully shaped beard"
};


/* expression leans neutral per framing */
static const char *const expressions[] = {
    "a warm, easy expression with the faintest hint of a smile",
    "a relaxed and friendly look, mouth softly closed",
    "a composed expression with a quiet confidence",
    "a direct gaze with open, welcoming eyes",
    "a calm, self-assured expression, lips just touched by a smile",
    "an engaged, attentive look without grinning",
    "a soft, amiable expression — present but not performing",
    "a settled, poised expression, warm around the eyes",
    "a steady, confident gaze with a trace of humor at the mouth",
    "a friendly neutral expression, eyes bright and alert",
    "a grounded, approachable look",
    "an unforced, pleasant expression with a hint of a smile at one corner of the mouth",
    "a quietly confident expression, eyes engaged with the camera",
    "a warm and present expression, neither smiling broadly nor serious",
    "an open, natural look — gently smiling with the eyes"
};

/* clothing — each tagged 'u' (unisex), 'm' (male-read), or 'f' (female-read) */
typedef struct { const char *text; char sex; } Clothing;

static const Clothing clothing_formal[] = {
    {"a charcoal wool suit with a crisp white shirt",'u'},
    {"a navy blazer over a collared shirt",'u'},
    {"a dark turtleneck under a tailored jacket",'u'},
    {"a simple black cardigan over a button-down",'u'},
    {"a gray pinstripe jacket and open-collar shirt",'u'},
    {"a tweed sport coat over a pale shirt",'u'},
    {"a crisp white blouse beneath a dark blazer",'f'},
    {"a silk scarf knotted over a fitted jacket",'f'},
    {"a plum velvet jacket over a cream blouse",'f'},
    {"a wool vest over a pressed shirt",'u'},
    {"a three-piece brown suit",'m'},
    {"a black tuxedo with a bow tie",'m'},
    {"a burgundy waistcoat over a white shirt",'m'},
    {"a beige trench coat over a dark sweater",'u'},
    {"a fitted black dress with a thin gold chain",'f'},
    {"a navy cocktail dress",'f'},
    {"a cream silk blouse buttoned to the throat",'f'},
    {"a collared jacket with epaulets",'u'},
    {"a tailored blazer over a black camisole",'f'},
    {"an embroidered shawl over a simple dress",'f'},
    {"a high-necked lace blouse",'f'},
    {"a satin evening gown",'f'},
    {"a pressed dress shirt with a loosened tie",'m'},
    {"a dinner jacket in midnight blue",'m'},
    {"a long sheath dress in emerald green",'f'}
};
static const Clothing clothing_casual[] = {
    {"a faded denim jacket over a plain t-shirt",'u'},
    {"a gray crewneck sweater",'u'},
    {"a soft flannel shirt in muted reds",'u'},
    {"a worn-in henley, sleeves pushed back",'u'},
    {"a cream linen blouse",'f'},
    {"a chunky oatmeal-colored knit sweater",'u'},
    {"a loose chambray shirt",'u'},
    {"a faded band t-shirt under an open overshirt",'u'},
    {"a forest-green cable-knit sweater",'u'},
    {"a simple cotton tunic",'u'},
    {"a corduroy shirt in warm brown",'u'},
    {"a thin wool cardigan over a tank top",'u'},
    {"a striped sailor-cut shirt",'u'},
    {"a turtleneck in deep burgundy",'u'},
    {"a plain black t-shirt",'u'},
    {"a plain white t-shirt",'u'},
    {"a cropped sweatshirt",'f'},
    {"a tie-dyed tee",'u'},
    {"an oversized plaid shirt",'u'},
    {"a sleeveless tank top",'u'},
    {"a denim shirt buttoned halfway",'u'},
    {"a mustard-yellow sweater",'u'},
    {"a rust-colored henley",'u'},
    {"a faded floral summer dress",'f'},
    {"a cotton sundress",'f'},
    {"a lace-collared blouse",'f'},
    {"a Peter Pan collared top",'f'},
    {"a paisley-print shirt",'u'},
    {"a knit poncho in earthy tones",'u'},
    {"a worn leather jacket over a tee",'u'},
    {"a baggy wool sweater",'u'},
    {"a simple cardigan in forest green",'u'},
    {"a silk camisole with thin straps",'f'},
    {"a button-up shirt with the sleeves rolled to the elbow",'u'},
    {"a wrap top in deep teal",'f'},
    {"a pale blue Oxford shirt",'u'},
    {"a simple gray pullover",'u'},
    {"a faded green t-shirt",'u'},
    {"a navy striped long-sleeved tee",'u'},
    {"a heather-purple v-neck tee",'u'},
    {"a short-sleeved button-up in pale yellow",'u'},
    {"a thin zippered track jacket",'u'},
    {"a gray raglan sweatshirt with navy sleeves",'u'},
    {"a threadbare concert tee",'u'},
    {"a cream fisherman's-style knit sweater",'u'},
    {"a soft pink tee",'u'},
    {"a dark green polo shirt",'u'},
    {"a burgundy sweatshirt",'u'},
    {"a thin cotton henley in stone gray",'u'},
    {"an unbuttoned flannel over a white undershirt",'u'},
    {"a boxy oatmeal-colored cardigan",'u'},
    {"a denim vest over a plain tee",'u'},
    {"a checkered shirt tucked loosely at the waist",'u'},
    {"a mock-neck cotton top",'u'},
    {"a faded olive-green jacket",'u'},
    {"a light cotton kimono-style jacket, open",'u'},
    {"a tan windbreaker",'u'},
    {"a quilted vest over a long-sleeved shirt",'u'},
    {"a gingham button-up shirt",'u'},
    {"a navy fleece pullover",'u'},
    {"a plain ribbed turtleneck in cream",'u'},
    {"a thin crewneck in sky blue",'u'},
    {"a striped rugby shirt",'u'},
    {"a soft cotton sweatshirt with a faded graphic",'u'},
    {"a cropped denim jacket over a t-shirt",'u'},
    {"a linen shirt rumpled and untucked",'u'},
    {"a patterned button-down in muted blues",'u'},
    {"a cotton sweater with elbow patches",'u'},
    {"a faded bomber jacket zipped halfway",'u'},
    {"a wrap-front blouse in dusty rose",'f'},
    {"an off-the-shoulder knit top",'f'},
    {"a scoop-neck tee in heather gray",'u'},
    {"a crocheted vest over a white tank",'f'},
    {"a boat-neck striped top",'f'},
    {"a peasant blouse with loose sleeves",'f'},
    {"a sundress with thin straps and a floral print",'f'},
    {"a plaid pinafore over a cream blouse",'f'},
    {"a lightweight kaftan",'f'},
    {"a ribbed tank top layered under an open shirt",'u'},
    {"a varsity jacket with wool sleeves",'u'},
    {"a chunky turtleneck in charcoal",'u'},
    {"a long-sleeved crewneck in forest green",'u'},
    {"a soft lavender cardigan",'u'},
    {"a muted teal thermal shirt",'u'},
    {"a rust-orange crewneck sweater",'u'},
    {"a plain navy sweatshirt",'u'},
    {"a pale peach cotton blouse",'f'},
    {"a black ribbed bodysuit under an open shirt",'f'},
    {"a slouchy dolman-sleeve top",'f'},
    {"a soft gray cashmere sweater",'u'},
    {"a weather-beaten barn coat over a flannel",'u'},
    {"a simple crochet top",'f'},
    /* plain colored t-shirts */
    {"a plain red t-shirt",'u'},{"a plain navy t-shirt",'u'},
    {"a plain forest-green t-shirt",'u'},{"a plain charcoal t-shirt",'u'},
    {"a plain olive t-shirt",'u'},{"a plain burgundy t-shirt",'u'},
    {"a plain mustard-yellow t-shirt",'u'},{"a plain teal t-shirt",'u'},
    {"a plain burnt-orange t-shirt",'u'},{"a plain cream t-shirt",'u'},
    {"a plain sky-blue t-shirt",'u'},{"a plain dusty-pink t-shirt",'u'},
    {"a plain maroon t-shirt",'u'},{"a plain sage-green t-shirt",'u'},
    {"a faded vintage t-shirt",'u'},{"a pocket t-shirt in heather gray",'u'},
    /* flannels */
    {"a red and black buffalo-plaid flannel shirt",'u'},
    {"a green and navy plaid flannel shirt",'u'},
    {"a faded blue plaid flannel shirt",'u'},
    {"a brown and cream plaid flannel shirt",'u'},
    {"a worn flannel shirt in autumn reds and oranges",'u'},
    {"a heavy flannel shirt over a white tee",'u'},
    {"a plaid flannel shirt with the sleeves rolled up",'u'},
    {"a soft flannel shirt in forest greens",'u'},
    {"a gray and burgundy plaid flannel",'u'},
    /* sweaters / sweatshirts */
    {"a crewneck sweater in cobalt blue",'u'},
    {"a crewneck sweater in dusty rose",'u'},
    {"a crewneck sweater in hunter green",'u'},
    {"a crewneck sweater in cranberry red",'u'},
    {"a crewneck sweater in camel tan",'u'},
    {"a crewneck sweater in pale yellow",'u'},
    {"a cable-knit sweater in ivory",'u'},
    {"a cable-knit sweater in navy",'u'},
    {"a cable-knit sweater in charcoal",'u'},
    {"a college sweatshirt in faded maroon",'u'},
    {"a college sweatshirt in navy and gold",'u'},
    {"a vintage sweatshirt in sun-faded red",'u'},
    {"a pullover sweater in mustard yellow",'u'},
    {"a chunky cardigan in oatmeal",'u'},
    {"a chunky cardigan in rust",'u'},
    /* button-ups / Oxfords / henleys / polos */
    {"a pale blue Oxford shirt",'u'},
    {"a white Oxford shirt",'u'},
    {"a pink Oxford shirt",'u'},
    {"a chambray shirt in washed blue",'u'},
    {"a faded denim button-up",'u'},
    {"a red gingham button-up",'u'},
    {"a blue gingham button-up",'u'},
    {"a navy henley",'u'},
    {"a forest-green henley",'u'},
    {"a cream henley",'u'},
    {"a maroon henley",'u'},
    {"a navy polo shirt",'u'},
    {"a white polo shirt",'u'},
    {"a red polo shirt",'u'},
    {"a pale yellow polo shirt",'u'},
    /* turtlenecks / tank tops / jackets */
    {"a black turtleneck",'u'},
    {"a cream turtleneck",'u'},
    {"a forest-green turtleneck",'u'},
    {"a white tank top",'u'},
    {"a black tank top",'u'},
    {"a gray tank top",'u'},
    {"a classic blue denim jacket",'u'},
    {"a faded black denim jacket",'u'},
    {"a brown leather jacket",'u'},
    {"a green canvas field jacket",'u'},
    {"a navy windbreaker",'u'},
    {"a khaki chore coat",'u'},
    /* blouses / tops (female-leaning) */
    {"a white eyelet blouse",'f'},
    {"a floral print blouse",'f'},
    {"a ruffled blouse in soft pink",'f'},
    {"a blouse in dusty lavender",'f'},
    {"a crop top in burgundy",'f'},
    {"a scoop-neck tee in navy",'f'},
    /* more women's tops */
    {"a silk camisole in champagne",'f'},
    {"a silk camisole in black",'f'},
    {"a satin blouse in blush pink",'f'},
    {"a chiffon blouse in pale blue",'f'},
    {"a peasant blouse with embroidered neckline",'f'},
    {"a tie-neck blouse in cream",'f'},
    {"a bell-sleeved top in rust orange",'f'},
    {"a puff-sleeved blouse in white",'f'},
    {"a wrap blouse in forest green",'f'},
    {"a wrap top in plum",'f'},
    {"a smocked top in sky blue",'f'},
    {"a ribbed knit top in dusty rose",'f'},
    {"a square-neck top in sage green",'f'},
    {"a scoop-neck top in lilac",'f'},
    {"a V-neck blouse in mustard",'f'},
    {"a halter-neck top in coral",'f'},
    {"a cami top in ivory",'f'},
    {"a tank top in rose pink",'f'},
    {"a boho peasant top in cream",'f'},
    {"a lace-trimmed blouse in white",'f'},
    {"a polka-dot blouse in navy and white",'f'},
    {"a floral sundress in yellow and white",'f'},
    {"a floral sundress in blue",'f'},
    {"a gingham sundress in pink and white",'f'},
    {"a midi dress in sage green",'f'},
    {"a wrap dress in navy with white dots",'f'},
    {"a shirt dress in khaki",'f'},
    {"a denim dress with buttons down the front",'f'},
    {"a little black dress",'f'},
    {"a knit sweater dress in camel",'f'},
    {"a fit-and-flare dress in teal",'f'},
    {"a corduroy pinafore over a white tee",'f'},
    {"an overall dress in denim",'f'},
    {"a kimono-sleeve top in navy",'f'},
    {"a crochet top over a camisole",'f'},
    {"an embroidered peasant blouse in white",'f'},
    {"a lace-collared dress in navy",'f'},
    {"a sweater in blush pink",'f'},
    {"a mohair sweater in powder blue",'f'},
    {"an angora sweater in cream",'f'},
    {"a fuzzy cardigan in pale pink",'f'},
    {"a cropped cardigan in butter yellow",'f'},
    {"a long cardigan in heather gray",'f'},
    {"a striped boatneck top in navy and white",'f'},
    {"a mock-neck blouse in ivory",'f'},
    {"a flannel shirtdress in red plaid",'f'}
};
static const Clothing clothing_unusual[] = {
    {"a heavy knit Aran jumper",'u'},
    {"a wool duffel coat with toggle buttons",'u'},
    {"a brocade vest over a ruffled shirt",'u'},
    {"a kimono-style robe in indigo",'u'},
    {"a poncho woven in red and black",'u'},
    {"a dashiki in bright green and gold",'u'},
    {"a linen kurta",'m'},
    {"a sherwani in deep maroon",'m'},
    {"a cheongsam in black silk",'f'},
    {"a thick fur-lined parka with the hood down",'u'},
    {"a long black robe with wide sleeves",'u'},
    {"a traditional embroidered vest over a white shirt",'u'},
    {"a sari draped over one shoulder",'f'},
    {"a flamenco-cut blouse with ruffles",'f'},
    {"a salwar kameez in pale blue",'f'},
    {"a kimono in cherry blossom pattern",'f'},
    {"a yukata in dark blue",'u'},
    {"a hanbok jeogori in pink and white",'f'},
    {"an áo dài in sky blue",'f'},
    {"a cheongsam in red with gold embroidery",'f'},
    {"a tang suit jacket in black silk",'u'},
    {"a silk kurta with gold trim",'m'},
    {"a sherwani in ivory with embroidery",'m'},
    {"a dupatta draped over the shoulders",'f'},
    {"a nehru jacket in charcoal",'m'},
    {"an agbada in cream and gold",'m'},
    {"a boubou in royal blue",'u'},
    {"a kente-cloth stole over a plain shirt",'u'},
    {"a thobe in white",'m'},
    {"a djellaba in earth tones",'u'},
    {"a kaftan with gold embroidery",'u'},
    {"a caftan in turquoise",'f'},
    {"a huipil with floral embroidery",'f'},
    {"a guayabera shirt in white",'m'},
    {"a Western shirt with pearl snaps",'u'},
    {"a cowboy shirt with an embroidered yoke",'u'},
    {"a bolo tie over a pressed shirt",'m'},
    {"a charro jacket with silver buttons",'m'},
    {"a serape draped over the shoulders",'u'},
    {"a Peruvian poncho in earthy tones",'u'},
    {"a gaucho-style poncho",'u'},
    {"a Nordic lopapeysa with a patterned yoke",'u'},
    {"an Icelandic knit sweater with a geometric yoke",'u'},
    {"a fair-isle patterned sweater",'u'},
    {"a vyshyvanka with red cross-stitch",'u'},
    {"a kosovorotka shirt with embroidered collar",'m'},
    {"a Tibetan chuba in deep red",'u'},
    {"a batik shirt in indigo and white",'u'},
    {"an ikat-print top",'u'},
    {"a Hawaiian aloha shirt in a tropical print",'u'},
    {"a Hawaiian muumuu in a floral print",'f'},
    {"a tea-length dress in a 1950s cut",'f'},
    {"a pinstripe zoot-style jacket",'m'},
    {"a velvet smoking jacket in burgundy",'m'},
    {"a brocade jacket in forest green",'u'},
    {"a silk robe with a sash",'u'},
    {"a Chinese-style padded jacket",'u'},
    {"a Mongolian deel in deep blue",'u'},
    {"a shearling-lined denim jacket",'u'},
    {"a sequined top in silver",'f'},
    {"a beaded bolero jacket over a plain top",'f'},
    {"an embroidered Mexican blouse",'f'},
    {"a gele head-wrap paired with a plain blouse",'f'},
    {"a turban of patterned silk",'u'},
    {"a crocheted granny-square vest",'u'},
    {"a tie-dye caftan",'u'},
    {"a batik-print wrap top",'f'},
    {"a Breton fisherman's cap over a striped shirt",'u'},
    {"a Russian sarafan with embroidered bodice",'f'},
    {"a Scottish tartan sash over a white blouse",'f'},
    {"a Balinese embroidered kebaya",'f'},
    {"a Filipino barong tagalog in sheer piña fabric",'m'},
    {"a kilt-style sporran strap crossing the chest over a collarless shirt",'m'},
    {"a Bavarian dirndl bodice laced up the front",'f'},
    {"a lederhosen-style embroidered shirt",'m'},
    {"a Basque red beret with a black shirt",'u'},
    {"a Moroccan kaftan with intricate beadwork",'f'},
    {"a Nigerian iro and buba",'f'},
    {"a Rajasthani turban in saffron",'m'},
    {"a Sikh dastar in deep blue",'m'},
    {"a keffiyeh draped over the shoulders",'u'},
    {"a prayer shawl with fringed edges",'u'},
    {"a tallit over a white shirt",'m'}
};
static const Clothing clothing_bare[] = {
    {"nothing — the bare shoulders and upper chest are visible",'u'},
    {"no clothing, the bare shoulders exposed",'u'},
    {"a bath towel wrapped loosely across the chest",'u'},
    {"a bedsheet draped around the shoulders",'u'},
    {"only a thin chain at the neck, the upper chest bare",'u'},
    {"bare-shouldered, the torso unclothed",'u'},
    {"no shirt, bare collarbones and upper chest visible",'u'}
};

static const char *pick_clothing(const Clothing *arr, int n, char sex_c) {
    /* filter to entries matching sex or unisex */
    int idx[128]; int k = 0;
    for (int i = 0; i < n && k < 128; i++) {
        if (arr[i].sex == 'u' || arr[i].sex == sex_c) idx[k++] = i;
    }
    return arr[idx[rng_range(k)]].text;
}

static const char *pick_feat(const Feat *arr, int n, char sex_c, int attr) {
    const char *allowed;
    switch (attr) {
        case 0: case 1: allowed = "un"; break;
        case 2:         allowed = "n";  break;
        default:        allowed = "np"; break;
    }
    int idx[256], k = 0;
    for (int i = 0; i < n && k < 256; i++) {
        if ((arr[i].sex == 'u' || arr[i].sex == sex_c) && strchr(allowed, arr[i].qual)) idx[k++] = i;
    }
    if (k == 0) {
        for (int i = 0; i < n && k < 256; i++) {
            if (arr[i].sex == 'u' || arr[i].sex == sex_c) idx[k++] = i;
        }
    }
    return arr[idx[rng_range(k)]].text;
}

static const char *pick_hair_young(char skin_cat) {
    int n = sizeof hair_colors_young / sizeof *hair_colors_young;
    int idx[64], k = 0;
    for (int i = 0; i < n && k < 64; i++) {
        if (strchr(hair_colors_young[i].skins, skin_cat)) idx[k++] = i;
    }
    return hair_colors_young[idx[rng_range(k)]].text;
}

/* ---- generation ---- */

typedef enum { AGE_CHILD, AGE_TEEN, AGE_YA, AGE_ADULT, AGE_MID, AGE_ELDER } AgeBucket;
typedef enum { SEX_M, SEX_F } Sex;

static void usage(const char *p) {
    fprintf(stderr,
        "usage: %s [-s seed] [overrides]\n"
        "  -s, --seed N            PRNG seed (default: /dev/urandom)\n"
        "\n"
        "Overrides (each defaults to the seed-generated value):\n"
        "  --sex m|f               subject sex\n"
        "  --age N                 age in years\n"
        "  --attractiveness N      0-4\n"
        "  --clothing T            formal|casual|unusual|bare\n"
        "  --skin T                light|medium|dark\n"
        "  -h, --help              show this help\n",
        p);
}

int main(int argc, char **argv) {
    uint64_t seed = 0;
    int have_seed = 0;

    const char *o_sex = NULL, *o_age = NULL;
    const char *o_attr = NULL, *o_ctype = NULL, *o_skin = NULL;

    static struct option long_opts[] = {
        {"seed",           required_argument, 0, 's'},
        {"sex",            required_argument, 0, 1000},
        {"age",            required_argument, 0, 1001},
        {"attractiveness", required_argument, 0, 1014},
        {"clothing",       required_argument, 0, 1019},
        {"skin",           required_argument, 0, 1004},
        {"help",           no_argument,       0, 'h'},
        {0,0,0,0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "s:h", long_opts, NULL)) != -1) {
        switch (opt) {
            case 's': seed = strtoull(optarg, NULL, 0); have_seed = 1; break;
            case 1000: o_sex = optarg; break;
            case 1001: o_age = optarg; break;
            case 1014: o_attr = optarg; break;
            case 1019: o_ctype = optarg; break;
            case 1004: o_skin = optarg; break;
            case 'h': usage(argv[0]); return 0;
            default:  usage(argv[0]); return 2;
        }
    }
    if (!have_seed) {
        FILE *f = fopen("/dev/urandom", "rb");
        if (!f) { perror("/dev/urandom"); return 1; }
        if (fread(&seed, sizeof seed, 1, f) != 1) { perror("read"); fclose(f); return 1; }
        fclose(f);
    }
    rng_seed(seed);

    /* age — realistic skew toward adults, unless overridden */
    double age_w[] = { 0.07, 0.10, 0.22, 0.28, 0.20, 0.13 };
    AgeBucket age;
    int age_override_years = -1;
    if (o_age) {
        char *end = NULL;
        long n = strtol(o_age, &end, 10);
        if (!end || *end != 0 || n < 1 || n > 120) {
            fprintf(stderr, "bad --age: %s (expected integer years)\n", o_age);
            return 2;
        }
        age_override_years = (int)n;
        if      (n <= 12) age = AGE_CHILD;
        else if (n <= 17) age = AGE_TEEN;
        else if (n <= 27) age = AGE_YA;
        else if (n <= 39) age = AGE_ADULT;
        else if (n <= 59) age = AGE_MID;
        else              age = AGE_ELDER;
    } else {
        age = (AgeBucket)rng_weighted(age_w, 6);
    }
    int age_years;
    switch (age) {
        case AGE_CHILD: age_years = 5  + rng_range(8);  break;
        case AGE_TEEN:  age_years = 13 + rng_range(5);  break;
        case AGE_YA:    age_years = 18 + rng_range(10); break;
        case AGE_ADULT: age_years = 28 + rng_range(12); break;
        case AGE_MID:   age_years = 40 + rng_range(20); break;
        default:        age_years = 60 + rng_range(30); break;
    }

    Sex sex;
    if (o_sex) {
        if      (!strcmp(o_sex, "m") || !strcmp(o_sex, "male"))   sex = SEX_M;
        else if (!strcmp(o_sex, "f") || !strcmp(o_sex, "female")) sex = SEX_F;
        else { fprintf(stderr, "bad --sex: %s (want m|f|male|female)\n", o_sex); return 2; }
    } else {
        sex = rng_chance(0.5) ? SEX_M : SEX_F;
    }

    /* skin — two-stage: pick category by fixed weights, then uniform within category */
    double skin_w[] = { 0.55, 0.30, 0.15 };  /* L, M, D */
    char skin_cat;
    if (o_skin) {
        if      (!strcmp(o_skin, "light"))  skin_cat = 'L';
        else if (!strcmp(o_skin, "medium")) skin_cat = 'M';
        else if (!strcmp(o_skin, "dark"))   skin_cat = 'D';
        else { fprintf(stderr, "bad --skin: %s (want light|medium|dark)\n", o_skin); return 2; }
    } else {
        skin_cat = "LMD"[rng_weighted(skin_w, 3)];
    }
    int n_skin = sizeof skin_tones / sizeof *skin_tones;
    int matches[32], nm = 0;
    for (int i = 0; i < n_skin; i++) if (skin_tones[i].cat == skin_cat) matches[nm++] = i;
    int skin_i = matches[rng_range(nm)];
    const char *skin = skin_tones[skin_i].text;

    /* hair color gated on age, filtered by skin for young palette */
    const char *hair_color;
    if (age >= AGE_ELDER) {
        hair_color = pick(hair_colors_old, sizeof hair_colors_old / sizeof *hair_colors_old);
    } else if (age == AGE_MID && rng_chance(0.6)) {
        hair_color = pick(hair_colors_graying, sizeof hair_colors_graying / sizeof *hair_colors_graying);
    } else if (age == AGE_ADULT && rng_chance(0.15)) {
        hair_color = pick(hair_colors_graying, sizeof hair_colors_graying / sizeof *hair_colors_graying);
    } else {
        hair_color = pick_hair_young(skin_cat);
    }

    /* hair length — sex-correlated but not forced */
    int len;  /* 0 short, 1 medium, 2 long */
    if (sex == SEX_M) {
        double w[] = { 0.65, 0.28, 0.07 };
        len = rng_weighted(w, 3);
    } else {
        double w[] = { 0.18, 0.37, 0.45 };
        len = rng_weighted(w, 3);
    }
    const char *hairstyle;
    if (sex == SEX_M) {
        if (len == 0) hairstyle = pick(male_short,  sizeof male_short  / sizeof *male_short);
        else if (len == 1) hairstyle = pick(male_medium, sizeof male_medium / sizeof *male_medium);
        else hairstyle = pick(male_long, sizeof male_long / sizeof *male_long);
    } else {
        if (len == 0) hairstyle = pick(female_short,  sizeof female_short  / sizeof *female_short);
        else if (len == 1) hairstyle = pick(female_medium, sizeof female_medium / sizeof *female_medium);
        else hairstyle = pick(female_long, sizeof female_long / sizeof *female_long);
    }

    /* balding possibility for older men */
    int balding = 0;
    if (sex == SEX_M && age >= AGE_MID && rng_chance(age >= AGE_ELDER ? 0.5 : 0.25)) balding = 1;

    /* facial hair — adult men only, more common with age */
    const char *facial_hair = NULL;
    if (sex == SEX_M && age >= AGE_YA) {
        double p = (age == AGE_YA) ? 0.25 : (age == AGE_ADULT) ? 0.45 : 0.55;
        if (rng_chance(p)) facial_hair = pick(beards, sizeof beards / sizeof *beards);
    }

    /* attractiveness — most people look average, tails thinner; rolled early so features can filter */
    double aw[] = { 0.08, 0.22, 0.40, 0.22, 0.08 };
    int attr = rng_weighted(aw, 5);

    if (o_attr) {
        char *end2 = NULL;
        long n2 = strtol(o_attr, &end2, 10);
        if (!end2 || *end2 != 0 || n2 < 0 || n2 > 4) {
            fprintf(stderr, "bad --attractiveness: %s (want 0-4)\n", o_attr);
            return 2;
        }
        attr = (int)n2;
    }

    char sex_cf = (sex == SEX_M) ? 'm' : 'f';
    const char *eyes = pick_feat(eye_colors,  sizeof eye_colors  / sizeof *eye_colors,  sex_cf, attr);
    const char *face = pick_feat(face_shapes, sizeof face_shapes / sizeof *face_shapes, sex_cf, attr);
    const char *nose = pick_feat(nose_shapes, sizeof nose_shapes / sizeof *nose_shapes, sex_cf, attr);
    const char *lips = pick_feat(lip_shapes,  sizeof lip_shapes  / sizeof *lip_shapes,  sex_cf, attr);

    /* build — mild age correlation; attr also biases toward trim/athletic or heavy/gaunt */
    const char *build;
    double bw_young[]  = { 0.35, 0.50, 0.15 };
    double bw_adult[]  = { 0.25, 0.50, 0.25 };
    double bw_old[]    = { 0.35, 0.35, 0.30 };
    int bi;
    if (age <= AGE_TEEN) bi = rng_weighted(bw_young, 3);
    else if (age >= AGE_MID) bi = rng_weighted(bw_old, 3);
    else bi = rng_weighted(bw_adult, 3);
    if (attr >= 3) bi = 1;  /* attractive subjects always average/athletic */
    if (bi == 0) build = pick(builds_thin,  sizeof builds_thin  / sizeof *builds_thin);
    else if (bi == 1) build = pick(builds_avg, sizeof builds_avg / sizeof *builds_avg);
    else build = pick(builds_heavy, sizeof builds_heavy / sizeof *builds_heavy);
    const char *attr_phrase;
    static const Clothing attr0[] = {
        {"plain and unremarkable, with a face that could be called homely",'u'},
        {"unattractive in a quiet, forgettable way",'u'},
        {"homely, with features that do not flatter",'u'},
        {"awkward-featured and a little odd-looking",'u'},
        {"of rough, weathered looks",'u'}
    };
    static const Clothing attr1[] = {
        {"of modest, ordinary looks",'u'},
        {"a touch below average in appearance",'u'},
        {"plain but not unpleasant to look at",'u'},
        {"unremarkable but pleasant enough",'u'}
    };
    static const Clothing attr2[] = {
        {"average and everyday in appearance",'u'},
        {"of ordinary looks",'u'},
        {"plainly average-looking",'u'},
        {"unremarkable, with a face you might pass in the street and not recall",'u'},
        {"neither handsome nor homely",'m'},
        {"neither pretty nor plain",'f'}
    };
    static const Clothing attr3[] = {
        {"good-looking in an easy, approachable way",'u'},
        {"pleasant-faced and attractive",'u'},
        {"agreeable and nice to look at",'u'},
        {"above-average in looks without being showy",'u'},
        {"handsome in an understated way",'m'},
        {"ruggedly handsome",'m'},
        {"pretty in a quiet way",'f'},
        {"lovely in an understated way",'f'}
    };
    static const Clothing attr4[] = {
        {"strikingly attractive, with features that draw the eye",'u'},
        {"possessed of rare, striking beauty",'u'},
        {"arrestingly handsome",'m'},
        {"classically handsome",'m'},
        {"strikingly handsome, with features that draw the eye",'m'},
        {"unusually beautiful, with a face that stops conversation",'f'},
        {"classically beautiful",'f'},
        {"radiantly beautiful",'f'}
    };
    switch (attr) {
        case 0: attr_phrase = pick_clothing(attr0, sizeof attr0 / sizeof *attr0, (sex == SEX_M) ? 'm' : 'f'); break;
        case 1: attr_phrase = pick_clothing(attr1, sizeof attr1 / sizeof *attr1, (sex == SEX_M) ? 'm' : 'f'); break;
        case 2: attr_phrase = pick_clothing(attr2, sizeof attr2 / sizeof *attr2, (sex == SEX_M) ? 'm' : 'f'); break;
        case 3: attr_phrase = pick_clothing(attr3, sizeof attr3 / sizeof *attr3, (sex == SEX_M) ? 'm' : 'f'); break;
        default:attr_phrase = pick_clothing(attr4, sizeof attr4 / sizeof *attr4, (sex == SEX_M) ? 'm' : 'f'); break;
    }

    /* marks — rare-ish, stacked randomly */
    char marks_buf[512];
    marks_buf[0] = 0;
    int marks_n = 0;
    #define ADD_MARK(s) do { \
        if (marks_n++ > 0) strncat(marks_buf, ", ", sizeof marks_buf - strlen(marks_buf) - 1); \
        strncat(marks_buf, (s), sizeof marks_buf - strlen(marks_buf) - 1); \
    } while (0)
    {
        static const char *const freckles[] = {
            "a scattering of freckles across the nose and cheeks",
            "freckled cheeks and nose","a dense pattern of freckles across the face",
            "light freckling on the nose","sun-earned freckles across the bridge of the nose"
        };
        static const char *const moles[] = {
            "a small mole above the lip","a beauty mark on one cheek",
            "a small mole near the jaw","a dark mole beside the eye"
        };
        static const char *const scars[] = {
            "a faint old scar along one eyebrow","a thin pale scar on the chin",
            "a small scar through one eyebrow","an old scar faintly crossing the forehead"
        };
        static const char *const crow[] = {
            "fine lines at the corners of the eyes","faint crow's feet","soft laugh lines around the eyes",
            "the first creases at the outer eyes"
        };
        static const char *const deep[] = {
            "deep creases at the brow and mouth","deeply lined skin around the eyes and mouth",
            "weathered wrinkles across the face","a face lined by decades"
        };
        if (rng_chance(0.18)) ADD_MARK(pick(freckles, sizeof freckles / sizeof *freckles));
        if (rng_chance(0.08)) ADD_MARK(pick(moles, sizeof moles / sizeof *moles));
        if (rng_chance(0.06)) ADD_MARK(pick(scars, sizeof scars / sizeof *scars));
        if (age >= AGE_MID && rng_chance(0.7)) ADD_MARK(pick(crow, sizeof crow / sizeof *crow));
        if (age >= AGE_ELDER) ADD_MARK(pick(deep, sizeof deep / sizeof *deep));
    }

    /* glasses */
    double glasses_p = (age >= AGE_MID) ? 0.45 : (age >= AGE_YA) ? 0.25 : 0.10;
    const char *glasses = NULL;
    if (rng_chance(glasses_p)) {
        static const char *const g[] = {
            "thin wire-framed glasses","heavy black-framed glasses",
            "round tortoiseshell glasses","rimless reading glasses perched low on the nose",
            "thick horn-rimmed glasses","small oval-lensed glasses",
            "gold wire-rim glasses","half-moon reading glasses","tinted round glasses",
            "narrow rectangular glasses","steel-framed glasses"
        };
        glasses = pick(g, sizeof g / sizeof *g);
    }

    /* clothing picked from five buckets, filtered by sex */
    const char *clothing;
    int clothing_bare_chosen = 0;
    char sex_c = (sex == SEX_M) ? 'm' : 'f';
    if (age >= AGE_YA) {
        /* formal, casual, unusual, bare */
        double cw[] = { 0.20, 0.70, 0.05, 0.05 };
        int ci = rng_weighted(cw, 4);
        if (ci == 0) clothing = pick_clothing(clothing_formal,  sizeof clothing_formal  / sizeof *clothing_formal,  sex_c);
        else if (ci == 1) clothing = pick_clothing(clothing_casual,  sizeof clothing_casual  / sizeof *clothing_casual,  sex_c);
        else if (ci == 2) clothing = pick_clothing(clothing_unusual, sizeof clothing_unusual / sizeof *clothing_unusual, sex_c);
        else { clothing = pick_clothing(clothing_bare, sizeof clothing_bare / sizeof *clothing_bare, sex_c); clothing_bare_chosen = 1; }
    } else {
        /* children/teens: casual only */
        clothing = pick_clothing(clothing_casual, sizeof clothing_casual / sizeof *clothing_casual, sex_c);
    }

    const char *expression = pick(expressions, sizeof expressions / sizeof *expressions);

    /* ---- apply CLI overrides ---- */
    if (age_override_years >= 0) age_years = age_override_years;
    if (o_ctype) {
        char sc = (sex == SEX_M) ? 'm' : 'f';
        clothing_bare_chosen = 0;
        if (!strcmp(o_ctype, "formal"))
            clothing = pick_clothing(clothing_formal, sizeof clothing_formal / sizeof *clothing_formal, sc);
        else if (!strcmp(o_ctype, "casual"))
            clothing = pick_clothing(clothing_casual, sizeof clothing_casual / sizeof *clothing_casual, sc);
        else if (!strcmp(o_ctype, "unusual"))
            clothing = pick_clothing(clothing_unusual, sizeof clothing_unusual / sizeof *clothing_unusual, sc);
        else if (!strcmp(o_ctype, "bare")) {
            clothing = pick_clothing(clothing_bare, sizeof clothing_bare / sizeof *clothing_bare, sc);
            clothing_bare_chosen = 1;
        } else {
            fprintf(stderr, "bad --clothing: %s (want formal|casual|unusual|bare)\n", o_ctype);
            return 2;
        }
    }

    /* ---- emit prompt ---- */

    printf("A front-facing portrait photograph of a single person. The "
           "entire head must be visible inside the frame with clear empty "
           "space above the top of the hair — under no circumstances should "
           "the top of the head, the hair, or the crown be clipped by the "
           "upper edge. Leave generous headroom. The ears and chin should "
           "also sit comfortably inside the frame, not at the edges. The "
           "subject looks straight into the lens in a relaxed, neutral "
           "pose. ");

    const char *sex_noun;
    if (sex == SEX_M) {
        if (age == AGE_CHILD) sex_noun = "boy";
        else if (age == AGE_TEEN) sex_noun = "teenage boy";
        else sex_noun = "man";
    } else {
        if (age == AGE_CHILD) sex_noun = "girl";
        else if (age == AGE_TEEN) sex_noun = "teenage girl";
        else sex_noun = "woman";
    }

    printf("The subject is a %s %d years old, %s with %s skin and %s %s face. ",
           sex_noun, age_years, build, skin, article(face), face);

    printf("%s has %s eyes, %s %s nose, and %s lips, and is %s. ",
           (sex == SEX_M) ? "He" : "She", eyes, article(nose), nose, lips, attr_phrase);

    const char *He = (sex == SEX_M) ? "He" : "She";
    const char *His = (sex == SEX_M) ? "His" : "Her";

    if (balding) {
        static const char *const bt[] = {
            "%s hair is %s and thinning on top, what remains cut short. ",
            "%s has lost much of %s %s hair, keeping what is left close-cropped. ",
            "On top %s hair is thin and receding, %s color. "
        };
        int bi2 = rng_range(3);
        if (bi2 == 0) printf(bt[0], His, hair_color);
        else if (bi2 == 1) printf(bt[1], He, (sex == SEX_M) ? "his" : "her", hair_color);
        else printf(bt[2], His, hair_color);
    } else {
        static const char *const ht[] = {
            "%s hair is %s, worn in %s. ",
            "%s wears %s hair in %s. ",
            "%s has %s hair, styled as %s. "
        };
        int hi = rng_range(3);
        if (hi == 0) printf(ht[0], His, hair_color, hairstyle);
        else if (hi == 1) printf(ht[1], He, hair_color, hairstyle);
        else printf(ht[2], He, hair_color, hairstyle);
    }

    if (facial_hair) {
        static const char *const ft[] = { "He wears %s. ", "He has %s. ", "His face carries %s. " };
        printf(ft[rng_range(3)], facial_hair);
    }
    if (glasses) {
        static const char *const gt[] = { "%s wears %s. ", "%s has %s on. ", "On %s face sit %s. " };
        int gi = rng_range(3);
        if (gi == 2) printf(gt[2], (sex == SEX_M) ? "his" : "her", glasses);
        else printf(gt[gi], He, glasses);
    }

    if (marks_buf[0]) {
        static const char *const mt[] = { "Notable details: %s. ", "Other features: %s. ", "Also visible: %s. " };
        printf(mt[rng_range(3)], marks_buf);
    }

    if (clothing_bare_chosen) {
        printf("The subject is shown wearing %s. ", clothing);
    } else {
        static const char *const ct[] = {
            "%s is wearing %s. ", "%s is dressed in %s. ", "%s wears %s. "
        };
        printf(ct[rng_range(3)], He, clothing);
    }

    printf("The expression is %s.\n", expression);

    /* print the seed so runs are reproducible */
    printf("\n[seed: %llu]\n", (unsigned long long)seed);

    return 0;
}

#!/usr/bin/env python3
"""iudex.py — matricula output mittit ad OpenAI vision et rogat de aesthetica.

Usus:
    ./matricula                       # scribit matricula.ppm
    python3 iudex.py                  # legit matricula.ppm, mittit, scribit responsum
    python3 iudex.py -i foo.ppm
    python3 iudex.py --model gpt-4o   # alternum modellum

Requiritur: OPENAI_API_KEY in environment, `openai` et `Pillow` installed.
"""
import argparse
import base64
import io
import os
import sys

from PIL import Image
from openai import OpenAI


PROMPT = """You are an art director reviewing a sheet of 64 procedurally generated
stylized avatar tiles ("matricula"). Each tile depicts a fictional Roman-pagan
spirit or creature (archetypes: lar, manes, lemur, ianus, augur, quirinus, carna,
genius, flamen, faunus; species: humana, nympha, pygmaeus, gigas, penates, larva,
furia, satyrus). These are deterministic seeded procedural graphics — NOT
photographs, NOT real people. Rendering is cartoonish/stylized with post-effects
(patina, fresco, aurum, mosaicum, halftone, solarizatio, rimae, etc.). The
aesthetic is intentionally "ugly-cute" — do NOT suggest sterilizing it.

Your task is purely aesthetic: judge both the variety of procedural output
across axes like color, shape, and rendering AND the overall visual quality
of the sheet (coherence, craft, readability, whether tiles look "good"). Do
not attempt to identify or describe any individual tile — only aggregate
statistics (how many distinct clusters along each axis, and how well the sheet
holds together as art) matter.

Variety across the 64 tiles is an important criterion — axes where tiles look
repeatable/clustered are a flaw. But overall quality matters too: a sheet can
be varied yet ugly, or coherent yet boring. Balance both.

## Part 1 — Quantitative audit (REQUIRED, fill every row)

Count and score every axis below. For each row give:
- **n_distinct**: rough count of visually distinguishable clusters you see across 64 tiles (higher = better variety)
- **score**: 1-5 (1=severe monotony/clustering, 3=adequate, 5=excellent variety, many distinct tiles)
- **one-line note**: what's dominating or missing

Scoring rubric (base on n_distinct relative to feasible max for this axis):
- 1: ≤3 distinct clusters, heavy duplication
- 2: ~4 distinct, some clustering
- 3: ~5-6 distinct, adequate
- 4: ~7-8 distinct, strong variety
- 5: ≥9 distinct, excellent — nearly every tile reads uniquely on this axis

Output as a markdown table with columns: `axis | n_distinct | score | note`.

Required axes:
1. Skin tone (hue + lightness range)
2. Hair style (length, volume, shape)
3. Hair color
4. Beard presence & style (none / stubble / jaw / full / long / forked)
5. Eye shape (size, tilt, form)
6. Eye color
7. Face shape (oval/round/square/heart/etc.)
8. Expression (happy/sad/neutral/angry/etc.)
9. Teeth visibility & type
10. Ornament variety (laurel, helmet, band, cap, diadem, other)
11. Clothing variety
12. Background hue
13. Background luminosity (light vs dark)
14. Background texture/gradient direction
15. Post-effect type diversity (patina/fresco/halftone/mosaic/etc.)
16. Post-effect intensity spread
17. Species/gens spread (human vs phantastical)
18. Archetype spread

Then compute:
- **Variety score**: mean of row scores, 1 decimal
- **Quality score**: 1-5 holistic judgment of craft, coherence, readability, artistic appeal of the sheet
- **Overall score**: rough average of both, 1 decimal

Briefly justify the quality score (one sentence).

## Part 2 — Prioritized fixes (5-8 items)

For each issue:
- Aesthetic problem (monotony, clashing, muddy, dead space, etc.)
- Which axis from Part 1
- Concrete fix (parameter/weight/range)

End with one "biggest single win" recommendation tied to the lowest-scored axis."""


def ppm_to_png_bytes(path: str) -> bytes:
    """Lege PPM, converte in PNG bytes."""
    img = Image.open(path)
    buf = io.BytesIO()
    img.save(buf, format="PNG")
    return buf.getvalue()


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("-i", "--input", default="matricula.ppm",
                    help="imago PPM ad analyzandum [default: matricula.ppm]")
    ap.add_argument("--model", default="gpt-4o",
                    help="modellum OpenAI [default: gpt-4o]")
    ap.add_argument("-o", "--output", default=None,
                    help="scribe responsum in fasciculo (praeter stdout)")
    ap.add_argument("--detail", default="high", choices=["low", "high", "auto"])
    args = ap.parse_args()

    if not os.path.exists(args.input):
        print(f"error: {args.input} non invenitur", file=sys.stderr)
        return 1

    png_bytes = ppm_to_png_bytes(args.input)
    b64 = base64.b64encode(png_bytes).decode("ascii")
    data_url = f"data:image/png;base64,{b64}"

    client = OpenAI()
    resp = client.chat.completions.create(
        model=args.model,
        messages=[{
            "role": "user",
            "content": [
                {"type": "text", "text": PROMPT},
                {"type": "image_url",
                 "image_url": {"url": data_url, "detail": args.detail}},
            ],
        }],
    )
    text = resp.choices[0].message.content or ""

    print(text)
    if args.output:
        with open(args.output, "w") as f:
            f.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())

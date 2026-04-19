#!/usr/bin/env python3
import argparse
import base64
import os
import subprocess
import sys
from pathlib import Path

from openai import OpenAI
from PIL import Image

PROMPT = (
    'a representative from [redacted] presented this as their soul identity '
    'token (it does somewhat resemble them i am told). generate a realistic '
    'portrait of this entity. the entities are humanoids. the token is an '
    'abstract symbolic emblem, not a literal depiction — do not render it '
    'as wounds, scars, injuries, or markings on the skin. no text. '
)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('outdir')
    ap.add_argument('n', type=int)
    args = ap.parse_args()

    here = Path(__file__).resolve().parent
    matricula = here / 'matricula'
    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    client = OpenAI()

    total = {'input_tokens': 0, 'output_tokens': 0, 'total_tokens': 0,
             'input_text': 0, 'input_image': 0}

    for s in range(1, args.n + 1):
        ppm = outdir / f'{s}.ppm'
        png_out = outdir / f'{s}.png'

        print(f'[{s}/{args.n}] matricula semen={s}', flush=True)
        subprocess.run(
            [str(matricula), '-d', '1',
            # '-s', str(s),
            '-o', str(ppm)],
            check=True, stdout=subprocess.DEVNULL,
        )

        png_in = outdir / f'{s}.input.png'
        Image.open(ppm).save(png_in)

        print(f'[{s}/{args.n}] OpenAI images.edit', flush=True)
        with open(png_in, 'rb') as fh:
            resp = client.images.edit(
                model='gpt-image-1',
                image=fh,
                prompt=PROMPT,
                size='1024x1024',
            )
        png_out.write_bytes(base64.b64decode(resp.data[0].b64_json))
        png_in.unlink()

        u = getattr(resp, 'usage', None)
        if u is not None:
            it = getattr(u, 'input_tokens', 0) or 0
            ot = getattr(u, 'output_tokens', 0) or 0
            tt = getattr(u, 'total_tokens', 0) or 0
            det = getattr(u, 'input_tokens_details', None)
            itx = getattr(det, 'text_tokens', 0) if det else 0
            iim = getattr(det, 'image_tokens', 0) if det else 0
            total['input_tokens'] += it
            total['output_tokens'] += ot
            total['total_tokens'] += tt
            total['input_text'] += itx or 0
            total['input_image'] += iim or 0
            print(f'[{s}/{args.n}] usus: input={it} (text={itx} image={iim}) '
                  f'output={ot} total={tt}', flush=True)

    print(f'peractum: {args.n} imagines in {outdir}')
    print(f'usus summa: input={total["input_tokens"]} '
          f'(text={total["input_text"]} image={total["input_image"]}) '
          f'output={total["output_tokens"]} total={total["total_tokens"]}')


if __name__ == '__main__':
    main()

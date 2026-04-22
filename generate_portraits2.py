#!/usr/bin/env python3
import argparse
import base64
import os
import random
import subprocess
import sys
from pathlib import Path

from openai import OpenAI
from PIL import Image

PROMPT = (
    'a representative from [redacted] presented this as their soul identity '
    'token (it does somewhat resemble them i am told). generate a realistic '
    'portrait of this entity. the entities are humanoids. the token is an '
    'abstract symbolic emblem representing an ideative state of life — an '
    'inner archetype, temperament, or spiritual signature — NOT a literal '
    'depiction of the body. the humanoid subject itself must be rendered '
    'as a healthy, living, functional person: intact skin, normal living '
    'complexion, clear eyes, upright posture, breathing and alive. do NOT '
    'render the subject as a zombie, corpse, undead, wounded, decaying, '
    'sickly, or disfigured. do not translate emblem shapes, colors, or '
    'distortions into wounds, scars, injuries, lesions, discoloration, '
    'sunken features, or markings on the skin. the emblem informs mood, '
    'bearing, and aura, not anatomy. no text. '
)

DESCRIBE_PROMPT = (
    'describe this abstract symbolic emblem in thorough visual detail: '
    'shapes, colors, textures, composition, proportions, spatial '
    'arrangement, and any notable motifs. do not interpret meaning; '
    'describe only what is visually present. respond with the description '
    'only, no preamble.'
)

DESCRIBE_MODEL = 'gpt-4o'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('outdir')
    ap.add_argument('n', type=int)
    args = ap.parse_args()

    here = Path(__file__).resolve().parent
    portrait = here / 'portrait'
    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    client = OpenAI()

    total = {'input_tokens': 0, 'output_tokens': 0, 'total_tokens': 0,
             'input_text': 0, 'input_image': 0}

    for i in range(1, args.n + 1):
        s = random.randint(1, 2**31 - 1)
        ppm = outdir / f'{s}.ppm'
        png_out = outdir / f'{s}.png'

        print(f'[{i}/{args.n}] portrait semen={s}', flush=True)
        subprocess.run(
            [str(portrait), '-s', str(s), '-o', str(ppm)],
            check=True, stdout=subprocess.DEVNULL,
        )

        png_in = outdir / f'{s}.input.png'
        Image.open(ppm).save(png_in)

        b64_in = base64.b64encode(png_in.read_bytes()).decode('ascii')

        print(f'[{i}/{args.n}] OpenAI describe', flush=True)
        desc_resp = client.chat.completions.create(
            model=DESCRIBE_MODEL,
            messages=[{
                'role': 'user',
                'content': [
                    {'type': 'text', 'text': DESCRIBE_PROMPT},
                    {'type': 'image_url',
                     'image_url': {'url': f'data:image/png;base64,{b64_in}'}},
                ],
            }],
        )
        description = desc_resp.choices[0].message.content.strip()
        (outdir / f'{s}.description.txt').write_text(description)
        print(f'[{i}/{args.n}] descriptio: {description[:120]}...', flush=True)

        du = getattr(desc_resp, 'usage', None)
        if du is not None:
            dit = getattr(du, 'prompt_tokens', 0) or 0
            dot = getattr(du, 'completion_tokens', 0) or 0
            dtt = getattr(du, 'total_tokens', 0) or 0
            print(f'[{i}/{args.n}] usus descriptionis: input={dit} '
                  f'output={dot} total={dtt}', flush=True)

        full_prompt = PROMPT + ' the emblem is described as follows: ' + description

        print(f'[{i}/{args.n}] OpenAI images.generate', flush=True)
        resp = client.images.generate(
            model='gpt-image-1',
            prompt=full_prompt,
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
            print(f'[{i}/{args.n}] usus: input={it} (text={itx} image={iim}) '
                  f'output={ot} total={tt}', flush=True)

    print(f'peractum: {args.n} imagines in {outdir}')
    print(f'usus summa: input={total["input_tokens"]} '
          f'(text={total["input_text"]} image={total["input_image"]}) '
          f'output={total["output_tokens"]} total={total["total_tokens"]}')


if __name__ == '__main__':
    main()

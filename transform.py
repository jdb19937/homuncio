#!/usr/bin/env python3
import argparse
import base64
import io
from pathlib import Path

from openai import OpenAI
from PIL import Image

EFFECTS_DIR = Path(__file__).parent / 'effects'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-e', '--effect', required=True)
    ap.add_argument('input')
    ap.add_argument('output')
    args = ap.parse_args()

    prompt_path = EFFECTS_DIR / f'{args.effect}.txt'
    prompt = prompt_path.read_text().strip()

    in_path = Path(args.input)
    out_path = Path(args.output)

    with Image.open(in_path) as im:
        w, h = im.size
    if w != h:
        raise SystemExit(f'input image must be square, got {w}x{h}')
    if w > 1024:
        raise SystemExit(f'input image must be at most 1024x1024, got {w}x{h}')

    client = OpenAI()

    print(f'OpenAI images.edit effect={args.effect} size=1024x1024', flush=True)
    with open(in_path, 'rb') as fh:
        resp = client.images.edit(
            model='gpt-image-1',
            image=fh,
            prompt=prompt,
            size='1024x1024',
        )
    img_bytes = base64.b64decode(resp.data[0].b64_json)
    img = Image.open(io.BytesIO(img_bytes))
    if img.size != (w, h):
        img = img.resize((w, h), Image.LANCZOS)
    img.save(out_path, format='PNG')

    u = getattr(resp, 'usage', None)
    if u is not None:
        it = getattr(u, 'input_tokens', 0) or 0
        ot = getattr(u, 'output_tokens', 0) or 0
        tt = getattr(u, 'total_tokens', 0) or 0
        det = getattr(u, 'input_tokens_details', None)
        itx = getattr(det, 'text_tokens', 0) if det else 0
        iim = getattr(det, 'image_tokens', 0) if det else 0
        print(f'usus: input={it} (text={itx} image={iim}) '
              f'output={ot} total={tt}', flush=True)

    print(f'peractum: {out_path}')


if __name__ == '__main__':
    main()

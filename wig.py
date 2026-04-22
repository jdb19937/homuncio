#!/usr/bin/env python3
import argparse
import base64
from pathlib import Path

from openai import OpenAI
from PIL import Image

PROMPT = (
    'generate a new portrait of the exact same person in this image, '
    'preserving their face, features, skin tone, expression, and pose, '
    'but now wearing a traditional full-bottomed english judge wig '
    '(long, white, curled, powdered) as worn in british courts. '
    'keep the background and framing consistent. no text.'
)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('input')
    ap.add_argument('output')
    args = ap.parse_args()

    in_path = Path(args.input)
    out_path = Path(args.output)

    with Image.open(in_path) as im:
        w, h = im.size
    if (w, h) == (1024, 1024):
        size = '1024x1024'
    elif (w, h) == (1024, 1536):
        size = '1024x1536'
    elif (w, h) == (1536, 1024):
        size = '1536x1024'
    else:
        if w >= h * 1.2:
            size = '1536x1024'
        elif h >= w * 1.2:
            size = '1024x1536'
        else:
            size = '1024x1024'

    client = OpenAI()

    print(f'OpenAI images.edit size={size}', flush=True)
    with open(in_path, 'rb') as fh:
        resp = client.images.edit(
            model='gpt-image-1',
            image=fh,
            prompt=PROMPT,
            size=size,
        )
    out_path.write_bytes(base64.b64decode(resp.data[0].b64_json))

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

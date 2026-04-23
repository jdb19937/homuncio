#!/usr/bin/env python3
import argparse
import base64
import io
import os
import random
import subprocess
import sys
from pathlib import Path

from openai import OpenAI
from PIL import Image


def run_promptum(promptum_path: Path, seed: int, extra: list) -> str:
    """Invoke promptum -s <seed> [extra] and return the prompt text (seed trailer stripped)."""
    result = subprocess.run(
        [str(promptum_path), '-s', str(seed)] + extra,
        check=True, capture_output=True, text=True,
    )
    out = result.stdout
    lines = [ln for ln in out.splitlines() if not ln.startswith('[seed:')]
    return '\n'.join(ln for ln in lines if ln.strip()).strip()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-o', required=True, help='output PNG path')
    ap.add_argument('-d', type=int, default=512, help='square image side length (default 512)')
    ap.add_argument('-s', type=int, default=None, help='promptum seed (random if omitted)')
    ap.add_argument('--sex', help='m|f|male|female — forwarded to promptum')
    ap.add_argument('--age', help='integer years — forwarded to promptum')
    ap.add_argument('--attractiveness', help='0-4 — forwarded to promptum')
    ap.add_argument('--clothing', help='formal|casual|unusual|bare — forwarded to promptum')
    ap.add_argument('--skin', help='light|medium|dark — forwarded to promptum')
    args = ap.parse_args()

    extra = []
    for flag in ('sex', 'age', 'attractiveness', 'clothing', 'skin'):
        v = getattr(args, flag)
        if v is not None:
            extra += [f'--{flag}', v]

    here = Path(__file__).resolve().parent
    promptum = here / 'promptum'
    if not promptum.exists():
        print(f'promptum binary not found at {promptum}; build with: cc -O2 -o promptum promptum.c',
              file=sys.stderr)
        sys.exit(1)

    seed = args.s if args.s is not None else random.randint(1, 2**31 - 1)
    prompt = run_promptum(promptum, seed, extra)

    print(f'seed={seed}', file=sys.stderr)
    print(prompt, file=sys.stderr)

    client = OpenAI()
    resp = client.images.generate(
        model='gpt-image-1',
        prompt=prompt,
        size='1024x1024',
    )
    img_bytes = base64.b64decode(resp.data[0].b64_json)

    img = Image.open(io.BytesIO(img_bytes))
    if args.d != img.width or args.d != img.height:
        img = img.resize((args.d, args.d), Image.LANCZOS)
    img.save(args.o, format='PNG')

    u = getattr(resp, 'usage', None)
    if u is not None:
        it = getattr(u, 'input_tokens', 0) or 0
        ot = getattr(u, 'output_tokens', 0) or 0
        tt = getattr(u, 'total_tokens', 0) or 0
        det = getattr(u, 'input_tokens_details', None)
        itx = getattr(det, 'text_tokens', 0) if det else 0
        iim = getattr(det, 'image_tokens', 0) if det else 0
        print(f'usus: input={it} (text={itx} image={iim}) '
              f'output={ot} total={tt}', file=sys.stderr)

    print(f'wrote {args.o} ({args.d}x{args.d}) seed={seed}', file=sys.stderr)


if __name__ == '__main__':
    main()

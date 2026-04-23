#!/usr/bin/env python3
import argparse
import base64
import io
import random
import subprocess
import sys
from pathlib import Path

from openai import OpenAI
from PIL import Image


JUDGE_SYSTEM_PROMPT = (
    "You are a strict evaluator. You will be given a portrait-generation prompt "
    "(in text) and a single portrait image. Your job is to rate how well the "
    "image matches the prompt on an integer scale from 1 to 10. "
    "1 = the image contradicts most of the prompt; "
    "5 = partial match, several key traits wrong or missing; "
    "10 = every specific trait named in the prompt is clearly visible in the image. "
    "Consider sex, age, skin tone, hair color and style, facial hair, glasses, "
    "clothing, framing (head centered, no crown clipping), and overall attractiveness "
    "tier. Respond with the integer rating on the first line, then a brief "
    "one-or-two-sentence justification on the second line. No other text."
)


def run_promptum(promptum_path: Path, seed: int, extra: list) -> str:
    result = subprocess.run(
        [str(promptum_path), '-s', str(seed)] + extra,
        check=True, capture_output=True, text=True,
    )
    out = result.stdout
    lines = [ln for ln in out.splitlines() if not ln.startswith('[seed:')]
    return '\n'.join(ln for ln in lines if ln.strip()).strip()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-i', required=True, help='input image (must be square)')
    ap.add_argument('-s', type=int, default=None,
                    help='promptum seed (random if omitted)')
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
        print(f'promptum binary not found at {promptum}', file=sys.stderr)
        sys.exit(1)

    in_path = Path(args.i)
    with Image.open(in_path) as im:
        w, h = im.size
        if w != h:
            print(f'input image must be square, got {w}x{h}', file=sys.stderr)
            sys.exit(1)
        im_small = im.convert('RGB').resize((256, 256), Image.LANCZOS)

    buf = io.BytesIO()
    im_small.save(buf, format='PNG')
    b64_img = base64.b64encode(buf.getvalue()).decode('ascii')

    seed = args.s if args.s is not None else random.randint(1, 2**31 - 1)
    prompt = run_promptum(promptum, seed, extra)

    print(f'seed={seed}', file=sys.stderr)
    print(prompt, file=sys.stderr)
    print('', file=sys.stderr)

    client = OpenAI()

    resp = client.chat.completions.create(
        model='gpt-5.4',
        messages=[
            {'role': 'system', 'content': JUDGE_SYSTEM_PROMPT},
            {'role': 'user', 'content': [
                {'type': 'text',
                 'text': f'Rate how well this image matches the following prompt.\n\nPROMPT:\n{prompt}'},
                {'type': 'image_url',
                 'image_url': {'url': f'data:image/png;base64,{b64_img}'}},
            ]},
        ],
    )
    content = resp.choices[0].message.content.strip()
    print(content)

    u = getattr(resp, 'usage', None)
    if u is not None:
        it = getattr(u, 'prompt_tokens', 0) or 0
        ot = getattr(u, 'completion_tokens', 0) or 0
        tt = getattr(u, 'total_tokens', 0) or 0
        print(f'usus: input={it} output={ot} total={tt}', file=sys.stderr)


if __name__ == '__main__':
    main()

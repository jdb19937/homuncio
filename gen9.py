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


CELLS = [
    'TOP-LEFT',    'TOP-CENTER',    'TOP-RIGHT',
    'MIDDLE-LEFT', 'MIDDLE-CENTER', 'MIDDLE-RIGHT',
    'BOTTOM-LEFT', 'BOTTOM-CENTER', 'BOTTOM-RIGHT',
]

SYSTEM_PROMPT = (
    "You are an image generator. The user will send a single message containing "
    "nine portrait prompts, each labeled with the cell it belongs to in a 3x3 "
    "grid: TOP-LEFT, TOP-CENTER, TOP-RIGHT, MIDDLE-LEFT, MIDDLE-CENTER, "
    "MIDDLE-RIGHT, BOTTOM-LEFT, BOTTOM-CENTER, BOTTOM-RIGHT. Generate a SINGLE "
    "square image arranged as a 3x3 grid of nine portraits, placing each "
    "portrait in the cell named by its label. Each cell must contain exactly "
    "one of the nine portraits and must follow its own prompt's framing and "
    "subject-description rules independently. The nine cells depict nine "
    "different people. Use consistent photographic style and lighting across "
    "all nine so the grid reads as a coherent set. Do not add borders, gutters, "
    "labels, captions, text, or decoration — just the nine portraits tiled "
    "edge-to-edge."
)


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
    ap.add_argument('-d', type=int, default=1024,
                    help='square image side length (default 1024)')
    ap.add_argument('-s', type=int, default=None,
                    help='base seed; prompts use s, s+1, ..., s+8 (random if omitted)')
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

    base_seed = args.s if args.s is not None else random.randint(1, 2**31 - 1)
    seeds = [base_seed + i for i in range(9)]
    prompts = [run_promptum(promptum, s, extra) for s in seeds]

    for c, s, p in zip(CELLS, seeds, prompts):
        print(f'--- {c} (seed={s}) ---', file=sys.stderr)
        print(p, file=sys.stderr)
        print('', file=sys.stderr)

    client = OpenAI()

    user_msg = '\n\n'.join(
        f'=== {c} CELL ===\n{p}' for c, p in zip(CELLS, prompts)
    )

    input_msgs = [
        {'role': 'system', 'content': SYSTEM_PROMPT},
        {'role': 'user', 'content': user_msg},
    ]

    print('OpenAI responses.create with image_generation tool', file=sys.stderr)
    resp = client.responses.create(
        model='gpt-5.4',
        input=input_msgs,
        tools=[{'type': 'image_generation', 'size': '1024x1024'}],
    )

    image_b64 = None
    for item in resp.output:
        if getattr(item, 'type', None) == 'image_generation_call':
            image_b64 = getattr(item, 'result', None)
            if image_b64:
                break
    if not image_b64:
        print('no image returned in response', file=sys.stderr)
        sys.exit(1)

    img_bytes = base64.b64decode(image_b64)
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

    print(f'wrote {args.o} ({args.d}x{args.d}) seeds={seeds}', file=sys.stderr)


if __name__ == '__main__':
    main()

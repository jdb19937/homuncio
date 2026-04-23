#!/usr/bin/env python3
import argparse
import random
import re
import subprocess
import sys
import tempfile
import time
from pathlib import Path

from PIL import Image


def log(msg):
    print(f'[try16 +{time.time() - START:.1f}s] {msg}', file=sys.stderr, flush=True)


START = time.time()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-s', type=int, default=None,
                    help='base seed (random if omitted); cells use s, s+1, ..., s+15')
    ap.add_argument('--sex', help='m|f|male|female — forwarded')
    ap.add_argument('--age', help='integer years — forwarded')
    ap.add_argument('--attractiveness', help='0-4 — forwarded')
    ap.add_argument('--clothing', help='formal|casual|unusual|bare — forwarded')
    ap.add_argument('--skin', help='light|medium|dark — forwarded')
    args = ap.parse_args()

    extra = []
    for flag in ('sex', 'age', 'attractiveness', 'clothing', 'skin'):
        v = getattr(args, flag)
        if v is not None:
            extra += [f'--{flag}', v]

    base_seed = args.s if args.s is not None else random.randint(1, 2**31 - 1)
    log(f'base_seed={base_seed} overrides={extra or "(none)"}')

    here = Path(__file__).resolve().parent
    gen16 = here / 'gen16.py'
    judge = here / 'judge.py'
    if not gen16.exists() or not judge.exists():
        log('gen16.py or judge.py missing in script directory')
        sys.exit(1)

    with tempfile.TemporaryDirectory() as tmpdir:
        tmp = Path(tmpdir)
        matrix_path = tmp / 'matrix.png'

        log(f'step 1/3: calling gen16.py to produce 4x4 matrix -> {matrix_path.name}')
        t0 = time.time()
        subprocess.run(
            [str(gen16), '-o', str(matrix_path), '-s', str(base_seed)] + extra,
            check=True,
        )
        log(f'matrix generated in {time.time() - t0:.1f}s')

        log('step 2/3: cropping 16 cells to 256x256')
        with Image.open(matrix_path) as im:
            im = im.convert('RGB')
            w, h = im.size
            if w != h:
                log(f'gen16 output not square: {w}x{h}')
                sys.exit(1)
            cell_px = w // 4
            log(f'matrix is {w}x{h}, cell size {cell_px}x{cell_px}')

            cells = []
            for r in range(4):
                for c in range(4):
                    box = (c * cell_px, r * cell_px, (c + 1) * cell_px, (r + 1) * cell_px)
                    ci = im.crop(box).resize((256, 256), Image.LANCZOS)
                    cp = tmp / f'cell_r{r+1}c{c+1}.png'
                    ci.save(cp, format='PNG')
                    cells.append((r + 1, c + 1, cp))

        log('step 3/3: judging each cell')
        scores = []
        for i, (r, c, cp) in enumerate(cells):
            seed = base_seed + i
            log(f'[{i+1}/16] judging cell ({r},{c}) seed={seed}...')
            t0 = time.time()
            result = subprocess.run(
                [str(judge), '-i', str(cp), '-s', str(seed)] + extra,
                check=True, capture_output=True, text=True,
            )
            dt = time.time() - t0
            out = result.stdout.strip()
            m = re.match(r'\s*(\d+)', out)
            if not m:
                log(f'could not parse score from: {out!r}')
                sys.exit(1)
            score = int(m.group(1))
            if not (1 <= score <= 10):
                log(f'score out of range: {score}')
                sys.exit(1)
            scores.append(score)
            running = sum(scores) / len(scores)
            log(f'[{i+1}/16] cell ({r},{c}) -> score={score}  ({dt:.1f}s)  running_avg={running:.2f}')
            for line in out.splitlines():
                print(f'           | {line}', file=sys.stderr, flush=True)

        avg = sum(scores) / len(scores)
        log(f'done in {time.time() - START:.1f}s  scores={scores}')
        print(f'average: {avg:.2f}')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
import argparse
import random
import re
import subprocess
import sys
import tempfile
import time
from pathlib import Path


START = time.time()


def log(msg):
    print(f'[try1 +{time.time() - START:.1f}s] {msg}', file=sys.stderr, flush=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-s', type=int, default=None,
                    help='base seed (random if omitted); runs use s, s+1, ..., s+15')
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
    gen = here / 'gen.py'
    judge = here / 'judge.py'
    if not gen.exists() or not judge.exists():
        log('gen.py or judge.py missing in script directory')
        sys.exit(1)

    scores = []
    with tempfile.TemporaryDirectory() as tmpdir:
        tmp = Path(tmpdir)
        for i in range(16):
            seed = base_seed + i
            img_path = tmp / f'portrait_{i:02d}.png'

            log(f'[{i+1}/16] generating portrait seed={seed}...')
            t0 = time.time()
            subprocess.run(
                [str(gen), '-o', str(img_path), '-s', str(seed)] + extra,
                check=True,
            )
            gen_dt = time.time() - t0
            log(f'[{i+1}/16] generated in {gen_dt:.1f}s')

            log(f'[{i+1}/16] judging seed={seed}...')
            t0 = time.time()
            result = subprocess.run(
                [str(judge), '-i', str(img_path), '-s', str(seed)] + extra,
                check=True, capture_output=True, text=True,
            )
            judge_dt = time.time() - t0
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
            log(f'[{i+1}/16] score={score}  (judge {judge_dt:.1f}s)  running_avg={running:.2f}')
            for line in out.splitlines():
                print(f'           | {line}', file=sys.stderr, flush=True)

    avg = sum(scores) / len(scores)
    log(f'done in {time.time() - START:.1f}s  scores={scores}')
    print(f'average: {avg:.2f}')


if __name__ == '__main__':
    main()

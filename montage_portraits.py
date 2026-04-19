#!/usr/bin/env python3
import argparse
from pathlib import Path

from PIL import Image


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('outdir')
    ap.add_argument('-o', '--output', default=None,
                    help='via exitus PNG [default: <outdir>/montage.png]')
    ap.add_argument('-s', '--size', type=int, default=256,
                    help='magnitudo cellulae in pixellis [default: 256]')
    ap.add_argument('-p', '--pad', type=int, default=4,
                    help='spatium inter cellulas [default: 4]')
    args = ap.parse_args()

    outdir = Path(args.outdir)
    output = Path(args.output) if args.output else outdir / 'montage.png'

    pairs = []
    for ppm in sorted(outdir.glob('*.ppm'), key=lambda p: int(p.stem)):
        png = ppm.with_suffix('.png')
        if png.exists():
            pairs.append((int(ppm.stem), ppm, png))

    if not pairs:
        raise SystemExit(f'nulla paria in {outdir}')

    sz = args.size
    pad = args.pad
    w = 2 * sz + 3 * pad
    h = len(pairs) * (sz + pad) + pad

    canvas = Image.new('RGB', (w, h), (255, 255, 255))

    for i, (_, ppm, png) in enumerate(pairs):
        y = pad + i * (sz + pad)
        a = Image.open(ppm).convert('RGB').resize((sz, sz), Image.LANCZOS)
        b = Image.open(png).convert('RGB').resize((sz, sz), Image.LANCZOS)
        canvas.paste(a, (pad, y))
        canvas.paste(b, (2 * pad + sz, y))

    canvas.save(output)
    print(f'scriptum: {output} ({w}x{h}, {len(pairs)} paria)')


if __name__ == '__main__':
    main()

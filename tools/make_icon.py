"""
Forja -- draws the application icon: an anvil with two crossed hammers.
Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
Date: November 20, 2026
SPDX-License-Identifier: GPL-2.0-only

Engraving style: the anvil and hammers of the trade card of Elwell and Taylor, smiths of London
(c. 1766; British Museum, Heal 85.99, public domain), cut out of the card by the outline in
assets/icon_forja.json and drawn by tools/engraving.py: cream body, ink lines, flat tile.

Writes:
  assets/app.ico         the executable's icon, on the default (black) tile, 16-256 px
  assets/app256.png      the same, 256 px, for the documents
  assets/icon_layer.bin  the drawing without the tile colour, 256 px, which the window
                         lays on the tile colour of the chosen accent at run time:
                         "WYI1", uint16 size, then per pixel T, K, R, G, B (bytes) where the
                         pixel is  rgb = RGB + tile * K / 255,  alpha = T  (premultiplied)
  build/icon_sheet.png   a look at every accent and size
The layer comes from drawing the icon twice, on black and on white: whatever changes
between the two is how much of the tile shows through.

Run:  python tools/make_icon.py      (needs numpy, opencv-python, Pillow)
"""
import pathlib
import struct
import sys

import cv2
import numpy as np
from PIL import Image

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import engraving  # noqa: E402

S = engraving.S
M, col = engraving.motif(engraving.load(ROOT / "assets" / "icon_forja.json"))
tile = engraving.rounded_tile(200 / 1024)

on0 = engraving.compose(M, col, (0, 0, 0))
on1 = engraving.compose(M, col, (255, 255, 255))
k = np.clip((on1 - on0).mean(axis=2) / 255, 0, 1)         # how much of the tile shows through
fg = np.clip(on0, 0, 255)                                  # the drawing, premultiplied


def icon(rgb, size=S):
    """The icon on a tile of colour rgb, as an RGBA image of the given size."""
    pre = (fg + np.array(rgb, np.float32) * k[..., None]) * tile[..., None]
    rgba = np.dstack([pre, tile * 255])
    if size != S:
        rgba = cv2.resize(rgba, (size, size), interpolation=cv2.INTER_AREA)
    a = rgba[..., 3:4]
    rgb_ = np.where(a > 0, rgba[..., :3] * 255 / np.maximum(a, 1e-6), 0)
    return Image.fromarray(np.clip(np.dstack([rgb_, a]), 0, 255).round().astype(np.uint8), "RGBA")


BLACK = (0x22, 0x24, 0x2C)                     # the default accent's tile (ACCENTS[0] in forja.c)
(ROOT / "build").mkdir(exist_ok=True)
sizes = [256, 64, 48, 40, 32, 24, 20, 16]
imgs = [icon(BLACK, z) for z in sizes]
imgs[0].save(ROOT / "assets" / "app.ico", sizes=[(z, z) for z in sizes], append_images=imgs[1:])
imgs[0].save(ROOT / "assets" / "app256.png")

N = 256
small = lambda a: cv2.resize(a.astype(np.float32), (N, N), interpolation=cv2.INTER_AREA)
T = small(tile)
K = small(k * tile)
P = small(fg * tile[..., None])
layer = np.dstack([T * 255, K * 255, P]).round().clip(0, 255).astype(np.uint8)
(ROOT / "assets" / "icon_layer.bin").write_bytes(b"WYI1" + struct.pack("<H", N) + layer.tobytes())

# sheet: the accents of the window (tile colours), big and small
tiles = [BLACK, (0x2F, 0x6F, 0xE0), (0xB8, 0x86, 0x0B), (0x6D, 0x28, 0xD9), (0x0E, 0xA5, 0xE9), (0xEA, 0x58, 0x0C),
         (0xDC, 0x26, 0x26), (0xEA, 0xB3, 0x08), (0x16, 0xA3, 0x4A), (0x0D, 0x94, 0x88), (0xDB, 0x27, 0x77), (0xF1, 0xF5, 0xF9)]
sheet = Image.new("RGB", (6 * 170 + 20, 2 * 250 + 20), (246, 246, 248))
for i, t in enumerate(tiles):
    x, y = 20 + (i % 6) * 170, 20 + (i // 6) * 250
    big = icon(t, 150)
    sheet.paste(big, (x, y), big)
    sx = x
    for z in (48, 32, 16):
        im = icon(t, z)
        sheet.paste(im, (sx, y + 170 + (48 - z) // 2), im)
        sx += z + 10
sheet.save(ROOT / "build" / "icon_sheet.png")
print("icon: app.ico, app256.png, icon_layer.bin written")

"""
Forja -- the setup wizard's pictures (installer/wizard.bmp, installer/wizard-small.bmp).
Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
Date: November 10, 2026
SPDX-License-Identifier: GPL-2.0-only

The side panel: the icon's drawing on the default black, a soft glow of the forge's fire
behind it, the name and a line in the three languages' common script (Latin). The small
picture at the top right of the inner pages: the icon. Both at twice the base size (Inno
Setup scales them down on ordinary screens and keeps them sharp on high-DPI ones).
Run after the icon is drawn (assets/app256.png):  python tools/make_wizard.py   (needs Pillow, numpy)
"""
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parent.parent
icon = Image.open(ROOT / "assets" / "app256.png").convert("RGBA")
FONTS = Path("C:/Windows/Fonts")


def font(name, size):
    try:
        return ImageFont.truetype(str(FONTS / name), size)
    except OSError:
        return ImageFont.load_default()


# ---- side panel, 164 x 314 at 100 %: drawn at 2x
W, H = 328, 628
y = np.linspace(0, 1, H)[:, None, None]
top, bottom = np.array([42, 44, 54], float), np.array([14, 15, 19], float)
panel = Image.fromarray((top + (bottom - top) * y).repeat(W, axis=1).astype(np.uint8), "RGB")
# the glow of the fire, warm, behind the icon
glow = Image.new("L", (W, H), 0)
ImageDraw.Draw(glow).ellipse((W // 2 - 150, 120, W // 2 + 150, 420), fill=90)
glow = glow.filter(ImageFilter.GaussianBlur(60))
panel = Image.composite(Image.new("RGB", (W, H), (210, 120, 40)), panel, glow)

ic = icon.resize((232, 232), Image.LANCZOS)
shadow = Image.new("RGBA", (W, H), (0, 0, 0, 0))
sh = Image.new("L", (232, 232), 0)
ImageDraw.Draw(sh).rounded_rectangle((0, 0, 231, 231), 46, fill=150)
shadow.paste((0, 0, 0, 255), ((W - 232) // 2, 162), sh)
shadow = shadow.filter(ImageFilter.GaussianBlur(12))
panel = panel.convert("RGBA")
panel.alpha_composite(shadow)
panel.alpha_composite(ic, ((W - 232) // 2, 150))

d = ImageDraw.Draw(panel)
for text, f, yy, col in (("Forja", font("segoeuib.ttf", 50), 410, (255, 255, 255)),
                         ("passwords made by chance", font("segoeuii.ttf", 21), 492, (240, 196, 120))):
    w = d.textlength(text, font=f)
    d.text(((W - w) / 2, yy), text, font=f, fill=col)
# a thin gold rule between the name and the line
d.line(((W / 2 - 40, 484), (W / 2 + 40, 484)), fill=(200, 150, 70), width=2)
d.text((W / 2 - d.textlength("© 2026 Gabriel Castiglia", font=font("segoeui.ttf", 16)) / 2, 590),
       "© 2026 Gabriel Castiglia", font=font("segoeui.ttf", 16), fill=(150, 152, 160))
panel.convert("RGB").save(ROOT / "installer" / "wizard.bmp")

# ---- small picture, 55 x 58 at 100 %: drawn at 2x on white (the page's colour)
small = Image.new("RGB", (110, 116), (255, 255, 255))
ic = icon.resize((104, 104), Image.LANCZOS)
small.paste(ic, (3, 6), ic)
small.save(ROOT / "installer" / "wizard-small.bmp")
print("wizard: wizard.bmp, wizard-small.bmp written")

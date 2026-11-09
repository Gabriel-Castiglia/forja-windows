"""
Random Password Generator -- draws the application icon (assets/app.ico, assets/app256.png).
Author: Gabriel Castiglia - 2026-11-09

A white padlock on a blue rounded square, with three dots on the lock body standing for a
hidden password. Drawn at 1024 px and reduced to each icon size, so the edges stay smooth.
Run:  python tools/make_icon.py   (needs Pillow)
"""

from pathlib import Path
from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent.parent
ASSETS = HERE / "assets"

S = 1024
BLUE_TOP = (63, 125, 240)
BLUE_BOTTOM = (36, 84, 196)
WHITE = (255, 255, 255)


def draw(size_px: int) -> Image.Image:
    img = Image.new("RGBA", (S, S), (0, 0, 0, 0))

    # background: rounded square with a vertical gradient
    grad = Image.new("RGBA", (S, S))
    gd = ImageDraw.Draw(grad)
    for y in range(S):
        t = y / (S - 1)
        c = tuple(round(BLUE_TOP[i] + (BLUE_BOTTOM[i] - BLUE_TOP[i]) * t) for i in range(3))
        gd.line([(0, y), (S, y)], fill=c + (255,))
    mask = Image.new("L", (S, S), 0)
    ImageDraw.Draw(mask).rounded_rectangle([40, 40, S - 40, S - 40], radius=210, fill=255)
    img.paste(grad, (0, 0), mask)

    d = ImageDraw.Draw(img)

    # shackle: a thick arc over the body
    sw = 92
    d.arc([300, 170, 724, 594], start=180, end=360, fill=WHITE, width=sw)
    d.rectangle([300, 380, 300 + sw, 520], fill=WHITE)
    d.rectangle([724 - sw, 380, 724, 520], fill=WHITE)

    # body
    d.rounded_rectangle([220, 470, 804, 880], radius=70, fill=WHITE)

    # three dots: a hidden password
    r = 46 if size_px >= 32 else 58
    for cx in (372, 512, 652):
        d.ellipse([cx - r, 675 - r, cx + r, 675 + r], fill=BLUE_BOTTOM + (255,))

    return img.resize((size_px, size_px), Image.LANCZOS)


def main() -> None:
    ASSETS.mkdir(exist_ok=True)
    sizes = [16, 20, 24, 32, 40, 48, 64, 96, 128, 256]
    frames = [draw(s) for s in sizes]
    frames[-1].save(ASSETS / "app256.png")
    frames[-1].save(ASSETS / "app.ico", format="ICO", sizes=[(s, s) for s in sizes],
                    append_images=frames[:-1])
    print("written:", ASSETS / "app.ico", ASSETS / "app256.png")


if __name__ == "__main__":
    main()

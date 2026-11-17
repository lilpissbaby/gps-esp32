"""Genera prueba.jpg (240x160, JPEG básico) para comprobar que el ESP32 dibuja imágenes de la SD.

Uso: python make_test_jpg.py [carpeta_de_salida]
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

W, H = 240, 160
BARS = [("R", (255, 0, 0)), ("G", (0, 255, 0)), ("B", (0, 0, 255)), ("W", (255, 255, 255))]


def make(out_dir: Path) -> Path:
    img = Image.new("RGB", (W, H), (20, 20, 20))
    draw = ImageDraw.Draw(img)
    bar_w = W // len(BARS)
    for i, (label, color) in enumerate(BARS):
        draw.rectangle([i * bar_w, 0, (i + 1) * bar_w - 1, 99], fill=color)
        text_color = (0, 0, 0) if sum(color) > 400 else (255, 255, 255)
        draw.text((i * bar_w + bar_w // 2, 50), label, fill=text_color, anchor="mm",
                  font=ImageFont.load_default(size=28))
    draw.text((W // 2, 130), "JPG desde la SD: OK", fill=(255, 255, 255), anchor="mm",
              font=ImageFont.load_default(size=18))
    out_dir.mkdir(parents=True, exist_ok=True)
    out = out_dir / "prueba.jpg"
    img.save(out, "JPEG", quality=90, progressive=False)  # el decodificador no admite progresivos
    return out


if __name__ == "__main__":
    print(make(Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")))

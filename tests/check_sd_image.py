"""Comprueba que sd/prueba.jpg viene incluido y que el ESP32 lo puede dibujar."""
import sys
from pathlib import Path

from PIL import Image

path = Path(sys.argv[1])
assert path.is_file(), f"falta {path}: el README pide copiarlo a la SD"
with Image.open(path) as im:
    assert im.format == "JPEG", f"{path} no es JPEG"
    assert not im.info.get("progressive"), "JPEG progresivo: el ESP32 no lo dibuja"
    assert im.width <= 240 and im.height <= 320, f"{im.size} no cabe en la pantalla"
print(f"OK {path} {im.size}")

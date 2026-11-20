"""Genera las imágenes de las tres carcasas: 3/4 delantera, 3/4 trasera y corte con las piezas."""
import sys
from pathlib import Path

from PIL import Image

import carcasa as cz
from render import label, render

COLORS = {
    "compacta": ((62, 66, 74), (78, 82, 90)),
    "robusta": ((88, 96, 70), (72, 79, 58)),
    "ergonomica": ((52, 88, 112), (66, 102, 126)),
}
CAP = (232, 122, 44)
PIECE = {
    "placa perforada": (60, 150, 80), "pantalla (placa)": (30, 30, 34), "pantalla (cristal)": (20, 22, 26),
    "pantalla (pines)": (190, 170, 60), "ESP32": (35, 35, 40), "ESP32 USB-C": (180, 180, 185),
    "batería 803040": (200, 200, 205), "GPS M10 (28x28x8)": (220, 150, 90), "TP4056": (40, 90, 170),
    "TP4056 USB-C": (180, 180, 185), "lector SD": (40, 70, 160), "MT3608": (40, 110, 190),
    "interruptor": (30, 30, 30),
}


def mesh(m, color):
    v, f = cz.to_mesh(m)
    return (v, f, color)


def screen():
    W = cz.WINDOW
    z = cz.PERF_Z1 + 7.1
    return cz.box(-15.3, 15.3, W["cy"] - 20.4, W["cy"] + 20.4, z + 0.02, z + 0.12)


def main(out):
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    sheets = []
    for v in cz.VARIANTS:
        front, back, caps = cz.build(v)
        c_front, c_back = COLORS[v.name]
        comps = cz.components()
        glass = comps["pantalla (cristal)"]
        assembled = [mesh(front, c_front), mesh(back, c_back), mesh(glass, (18, 20, 24)),
                     mesh(screen(), (52, 132, 120))] + [mesh(c, CAP) for c in caps]
        bb = (front + back).bounding_box()
        size = f"{bb[3] - bb[0]:.0f} × {bb[4] - bb[1]:.0f} × {bb[5] - bb[2]:.0f} mm (con el asa)"
        a = label(render(assembled, (0.85, 0.45, 1.5)), v.name.capitalize(), size)
        b = render(assembled, (-0.9, 0.35, -1.4))
        half = cz.box(-80, 0, -80, 200, -80, 80)
        cut = [mesh(front ^ half, c_front), mesh(back ^ half, c_back)]
        for name, part in comps.items():
            p = part ^ half
            if not p.is_empty():
                cut.append(mesh(p, PIECE.get(name, (120, 120, 120))))
        for cap in caps:
            p = cap ^ half
            if not p.is_empty():
                cut.append(mesh(p, CAP))
        c = render(cut, (1.0, 0.18, 0.25))
        for im, suffix in ((a, "frente"), (b, "trasera"), (c, "corte")):
            im.save(out / f"{v.name}_{suffix}.png")
        sheets.append((a, b, c))
        print("ok", v.name)

    tile = 600
    sheet = Image.new("RGB", (tile * 3, tile * len(sheets)), (246, 245, 241))
    for row, ims in enumerate(sheets):
        for col, im in enumerate(ims):
            sheet.paste(im.resize((tile, tile), Image.LANCZOS), (col * tile, row * tile))
    sheet.save(out / "comparativa.png")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "img")

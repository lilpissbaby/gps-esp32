"""Carcasas del GPS portátil ESP32: tres variantes paramétricas (compacta, robusta, ergonómica).

Coordenadas en mm, origen en la esquina trasera-inferior-central de la cavidad interior:
  x = ancho (izquierda -, derecha +, mirando la pantalla de frente)
  y = alto (0 = fondo de la cavidad, abajo)
  z = grosor (0 = cara interior de la tapa trasera, crece hacia la pantalla)

Uso:  python carcasa.py <carpeta_salida>
Genera por variante: <variante>_frente.stl, <variante>_tapa.stl, <variante>_botones.stl
y comprueba que ninguna pieza electrónica choca con la carcasa.
"""
import sys
from dataclasses import dataclass, field
from pathlib import Path

import numpy as np
from manifold3d import CrossSection as C
from manifold3d import Manifold as M

SEG = 48

# ---------------------------------------------------------------- medidas de la cavidad
CAV_W = 40.0         # ancho interior
CAV_H = 112.0        # alto interior
CAV_D = 28.6         # fondo interior (lo fija el GPS M10 de 28 mm tumbado arriba)
SPLIT_Z = 10.4       # plano de unión tapa trasera / frente (entre las dos capas de módulos)

ZF = CAV_D                       # cara interior del frontal
PERF_Z0 = ZF - 9.2               # placa perforada (1,6 mm)
PERF_Z1 = ZF - 7.6
SWITCH_TOP = PERF_Z1 + 4.9       # pulsadores de 12x12 x 4,9 mm

WINDOW = dict(cx=0.0, cy=71.0, w=32.0, h=42.0)          # zona visible de la pantalla
BUTTONS = {"+": (-7.62, 30.0), "-": (7.62, 30.0), "OK": (-7.62, 14.76), "<": (7.62, 14.76)}
BOSSES = [(-17.2, 3.1), (17.2, 3.1), (-17.2, 108.0), (17.2, 108.0)]
BOSS_R = 2.6


def box(x0, x1, y0, y1, z0, z1):
    return M.cube((x1 - x0, y1 - y0, z1 - z0)).translate((x0, y0, z0))


def components():
    """Piezas electrónicas como cajas (medidas de mupsim con calibre)."""
    perf = box(-18.25, 18.25, 1.0, 101.0, PERF_Z0, PERF_Z1)
    # muescas en las esquinas inferiores para los tetones de los tornillos
    perf = perf - box(14.2, 20, 0, 6.1, 0, ZF) - box(-20, -14.2, 0, 6.1, 0, ZF)
    parts = {
        "placa perforada": perf,
        "pantalla (placa)": box(-18.5, 18.5, 38.0, 101.0, PERF_Z1 + 2.5, PERF_Z1 + 4.1),
        "pantalla (cristal)": box(-18.5, 18.5, 43.6, 95.4, PERF_Z1 + 4.1, PERF_Z1 + 7.1),
        "pantalla (pines)": box(-9.0, 9.0, 97.0, 99.5, PERF_Z1, PERF_Z1 + 2.5),
        "ESP32": box(-14.25, 14.25, 0.8, 54.3, 12.1, PERF_Z0),
        "ESP32 USB-C": box(-4.6, 4.6, -0.4, 7.5, 12.1, 15.3),
        "batería 803040": box(-14.6, 14.6, 56.0, 100.0, 10.6, 18.7),
        "GPS M10 (28x28x8)": box(-14.0, 14.0, 102.5, 110.5, 0.3, 28.3),
        "TP4056": box(-13.0, 4.6, 0.5, 29.1, 1.8, 6.1),
        "TP4056 USB-C": box(-8.8, 0.4, -0.4, 7.0, 2.8, 6.1),
        "lector SD": box(-0.5, 17.5, 32.0, 50.0, 3.4, 7.0),
        "MT3608": box(-8.6, 8.6, 54.0, 90.8, 0.3, 6.5),
        "interruptor": box(16.0, 19.5, 77.0, 85.6, 6.9, 10.3),
    }
    for name, (bx, by) in BUTTONS.items():
        parts[f"pulsador {name}"] = box(bx - 6, bx + 6, by - 6, by + 6, PERF_Z1, SWITCH_TOP)
    return parts


# ---------------------------------------------------------------- formas redondeadas
def rrect(x0, x1, y0, y1, radii):
    """Rectángulo 2D con un radio por esquina: (abajo-izq, abajo-der, arriba-der, arriba-izq)."""
    pts = [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]
    sgn = [(1, 1), (-1, 1), (-1, -1), (1, -1)]
    circles = []
    for (px, py), (sx, sy), r in zip(pts, sgn, radii):
        r = max(r, 0.05)
        circles.append(C.circle(r, SEG).translate((px + sx * r, py + sy * r)))
    return C.batch_hull(circles)


def rounded_box(x0, x1, y0, y1, z0, z1, top_front, top_back, side_front, side_back, front_corner):
    """Caja redondeada como intersección de tres prismas:
    top_*   radio de las aristas verticales (vista desde arriba), delante/detrás
    side_*  radio de las aristas horizontales de arriba/abajo (vista lateral), delante/detrás
    front_corner  radio de las esquinas vistas de frente."""
    a = rrect(x0, x1, -z1, -z0, (top_front, top_front, top_back, top_back))
    a = a.extrude(y1 - y0).transform(np.array([[1, 0, 0, 0], [0, 0, 1, y0], [0, -1, 0, 0]], float))
    b = rrect(y0, y1, z0, z1, (side_back, side_back, side_front, side_front))
    b = b.extrude(x1 - x0).transform(np.array([[0, 0, 1, x0], [1, 0, 0, 0], [0, 1, 0, 0]], float))
    c = rrect(x0, x1, y0, y1, (front_corner,) * 4).extrude(z1 - z0).translate((0, 0, z0))
    return a ^ b ^ c


def cyl_z(cx, cy, z0, z1, r, seg=SEG):
    return M.cylinder(z1 - z0, r, r, seg).translate((cx, cy, z0))


def cyl_x(cy, cz, x0, x1, r, seg=SEG):
    return M.cylinder(x1 - x0, r, r, seg).transform(
        np.array([[0, 0, 1, x0], [1, 0, 0, cy], [0, 1, 0, cz]], float))


def cyl_y(cx, cz, y0, y1, r, seg=SEG):
    return M.cylinder(y1 - y0, r, r, seg).transform(
        np.array([[0, 1, 0, cx], [0, 0, 1, y0], [1, 0, 0, cz]], float))


def slot_x(cy, cz, x0, x1, length_y, height_z):
    """Ranura con extremos redondos atravesando una pared lateral (eje x)."""
    r = height_z / 2
    s = C.batch_hull([C.circle(r, SEG).translate((cy - length_y / 2 + r, cz)),
                      C.circle(r, SEG).translate((cy + length_y / 2 - r, cz))])
    return s.extrude(x1 - x0).transform(np.array([[0, 0, 1, x0], [1, 0, 0, 0], [0, 1, 0, 0]], float))


def slot_y(cx, cz, y0, y1, width_x, height_z):
    """Hueco con extremos redondos atravesando la pared de abajo (eje y)."""
    r = height_z / 2
    s = C.batch_hull([C.circle(r, SEG).translate((cx - width_x / 2 + r, -cz)),
                      C.circle(r, SEG).translate((cx + width_x / 2 - r, -cz))])
    return s.extrude(y1 - y0).transform(np.array([[1, 0, 0, 0], [0, 0, 1, y0], [0, -1, 0, 0]], float))


# ---------------------------------------------------------------- variantes
@dataclass
class Variant:
    name: str
    wall: float
    cav_radii: tuple            # (top_front, top_back, side_front, side_back, front_corner) de la cavidad
    cap_protrusion: float = 1.2
    extras: list = field(default_factory=list)


VARIANTS = [
    Variant("compacta", 1.8, (0.5, 2.2, 0.5, 1.2, 4.2)),
    Variant("robusta", 2.6, (0.6, 0.6, 0.6, 0.6, 2.4), cap_protrusion=1.8,
            extras=["bumpers", "bezel", "guard", "grip", "rails", "acrylic"]),
    Variant("ergonomica", 2.2, (1.8, 7.8, 1.8, 2.8, 7.8),
            extras=["palm", "fingers"]),
]


def cavity(v):
    return rounded_box(-CAV_W / 2, CAV_W / 2, 0, CAV_H, 0, CAV_D, *v.cav_radii)


def outer(v, grow=0.0):
    w = v.wall + grow
    r = [x + w for x in v.cav_radii]
    return rounded_box(-CAV_W / 2 - w, CAV_W / 2 + w, -w, CAV_H + w, -w, CAV_D + w, *r)


def window_cut(v, z_front):
    """Ventana de la pantalla con chaflán hacia fuera."""
    W = WINDOW
    inner = rrect(W["cx"] - W["w"] / 2, W["cx"] + W["w"] / 2, W["cy"] - W["h"] / 2, W["cy"] + W["h"] / 2, (1.0,) * 4)
    t = 1.2
    outer_r = rrect(W["cx"] - W["w"] / 2 - t, W["cx"] + W["w"] / 2 + t,
                    W["cy"] - W["h"] / 2 - t, W["cy"] + W["h"] / 2 + t, (1.0 + t,) * 4)
    a = inner.extrude(z_front - ZF + 5).translate((0, 0, ZF - 2))
    b = M.batch_hull([inner.extrude(0.01).translate((0, 0, z_front - t)),
                      outer_r.extrude(0.01).translate((0, 0, z_front + 0.01))])
    return a + b + outer_r.extrude(3).translate((0, 0, z_front))


def lanyard(v):
    """Asa para el cordón arriba, en la parte de la tapa trasera."""
    top = CAV_H + v.wall
    w = 12 if "bumpers" in v.extras else 10
    lug = M.batch_hull([cyl_x(top + 2.8, 3.0, -w / 2, w / 2, 3.2),
                        box(-w / 2, w / 2, top - 4, top - 3.9, -v.wall + 0.6, 7.0)])
    hole = cyl_x(top + 2.8, 3.0, -w, w, 1.7)
    return lug, hole


def build(v):
    cav = cavity(v)
    out = outer(v)
    z_front = CAV_D + v.wall
    z_back = -v.wall

    solid = out
    # --- extras que añaden material por fuera
    if "bumpers" in v.extras:
        big = outer(v, grow=1.4)
        for sx in (-1, 1):
            for (y0, y1) in ((-10, 9), (CAV_H - 12, CAV_H + 10)):
                corner = box(12 if sx > 0 else -40, 40 if sx > 0 else -12, y0, y1, -20, 60)
                solid = solid + (big ^ corner)
    if "bezel" in v.extras:
        W = WINDOW
        ring = rrect(-W["w"] / 2 - 4, W["w"] / 2 + 4, W["cy"] - W["h"] / 2 - 4, W["cy"] + W["h"] / 2 + 4, (3,) * 4)
        solid = solid + ring.extrude(1.4 + 1).translate((0, 0, z_front - 1))
    if "guard" in v.extras:
        ring = rrect(-15.5, 15.5, 14.76 - 9.5, 30 + 9.5, (4,) * 4) - rrect(-14, 14, 14.76 - 8, 30 + 8, (3,) * 4)
        solid = solid + ring.extrude(1.2 + 1).translate((0, 0, z_front - 1))
    if "rails" in v.extras:
        for sx in (-1, 1):
            rail = M.batch_hull([cyl_y(sx * 12, z_back + 0.2, 18, 94, 1.6)])
            solid = solid + rail
    if "palm" in v.extras:
        palm = M.sphere(1, 64).scale((17, 44, 6.5)).translate((0, 45.0, z_back + 4.0))
        solid = M.batch_hull([solid, palm])
    lug, lug_hole = lanyard(v)
    solid = solid + lug

    # --- vaciar y añadir tetones de tornillo
    solid = solid - cav
    for (bx, by) in BOSSES:
        solid = solid + (cyl_z(bx, by, 0, CAV_D, BOSS_R) ^ out)

    # --- cortes
    cuts = [window_cut(v, z_front), lug_hole]
    if "acrylic" in v.extras:   # cajeado interior para un cristal de metacrilato de 1,2 mm
        W = WINDOW
        pocket = rrect(-W["w"] / 2 - 2, W["w"] / 2 + 2, W["cy"] - W["h"] / 2 - 2, W["cy"] + W["h"] / 2 + 2, (2,) * 4)
        cuts.append(pocket.extrude(1.2 + 0.5).translate((0, 0, ZF - 0.5)))
    for (bx, by) in BUTTONS.values():
        cuts.append(cyl_z(bx, by, ZF - 1, z_front + 5, 4.7))
    # puertos de abajo: USB-C del ESP32 (programar) y del TP4056 (cargar)
    cuts.append(slot_y(0.0, 13.7, -v.wall - 5, 1, 12.6, 7.0))
    cuts.append(slot_y(-4.2, 4.45, -v.wall - 5, 1, 12.6, 7.0))
    # derecha: ranura de la microSD e interruptor
    cuts.append(slot_x(41.0, 6.1, CAV_W / 2 - 4, CAV_W / 2 + v.wall + 8, 13.0, 2.2))
    cuts.append(slot_x(41.0, 6.1, CAV_W / 2 + v.wall - 1.0, CAV_W / 2 + v.wall + 8, 16.0, 6.0))   # hueco para la uña
    cuts.append(slot_x(81.3, 8.6, CAV_W / 2 - 1, CAV_W / 2 + v.wall + 8, 6.6, 2.8))
    if "grip" in v.extras:
        for i in range(4):
            y = 52 + i * 7.5
            for sx in (-1, 1):
                g = box(-60, 60, y - 0.8, y + 0.8, z_back + 4, z_front - 4)
                g = g ^ (outer(v, grow=3) - outer(v, grow=-0.8))
                cuts.append(g if True else None)
    if "fingers" in v.extras:
        for y in (28.0, 42.0, 56.0):
            cuts.append(cyl_x(y, z_back - 9.3, -40, 40, 8.0, 96))
    # tornillos: rosca en el frente, paso y avellanado en la tapa
    for (bx, by) in BOSSES:
        cuts.append(cyl_z(bx, by, SPLIT_Z - 0.01, SPLIT_Z + 9, 1.05, 24))      # piloto M2,5
        cuts.append(cyl_z(bx, by, -40, SPLIT_Z + 0.01, 1.45, 24))              # paso M2,5
        cuts.append(cyl_z(bx, by, -40, 0.6, 2.6, 32))                           # cabeza
    solid = solid - M.batch_boolean(cuts, __import__("manifold3d").OpType.Add)

    # --- partir en frente y tapa, con pestaña de encaje en la tapa
    front = solid ^ box(-80, 80, -80, 200, SPLIT_Z, 80)
    back = solid ^ box(-80, 80, -80, 200, -80, SPLIT_Z)
    lip_out = rrect(-CAV_W / 2 + 0.2, CAV_W / 2 - 0.2, 0.2, CAV_H - 0.2, (v.cav_radii[4] - 0.2,) * 4)
    lip_in = rrect(-CAV_W / 2 + 1.4, CAV_W / 2 - 1.4, 1.4, CAV_H - 1.4, (max(v.cav_radii[4] - 1.4, 0.3),) * 4)
    lip = (lip_out - lip_in).extrude(1.6).translate((0, 0, SPLIT_Z))
    keep_out = M.batch_boolean([c for c in components().values()] +
                               [cyl_z(bx, by, 0, CAV_D, BOSS_R + 0.4) for bx, by in BOSSES],
                               __import__("manifold3d").OpType.Add)
    lip = lip - keep_out - slot_y(0.0, 13.7, -10, 9, 13.6, 8.0)
    back = back + lip

    caps = build_caps(v, z_front)
    return front, back, caps


def symbol(name, cx, cy, z):
    t = 0.5
    if name == "+":
        return box(cx - 2.5, cx + 2.5, cy - 0.6, cy + 0.6, z, z + t) + box(cx - 0.6, cx + 0.6, cy - 2.5, cy + 2.5, z, z + t)
    if name == "-":
        return box(cx - 2.5, cx + 2.5, cy - 0.6, cy + 0.6, z, z + t)
    if name == "OK":
        return cyl_z(cx, cy, z, z + t, 2.4) - cyl_z(cx, cy, z - 1, z + t + 1, 1.4)
    tri = C.batch_hull([C.circle(0.3, 12).translate((cx - 2.2, cy)),
                        C.circle(0.3, 12).translate((cx + 1.8, cy + 2.2)),
                        C.circle(0.3, 12).translate((cx + 1.8, cy - 2.2))])
    return tri.extrude(t).translate((0, 0, z))


def build_caps(v, z_front):
    caps = []
    top = z_front + v.cap_protrusion
    for name, (bx, by) in BUTTONS.items():
        flange = cyl_z(bx, by, SWITCH_TOP, ZF - 0.5, 6.0)
        stem = cyl_z(bx, by, ZF - 0.5, top, 4.3)
        caps.append(flange + stem + symbol(name, bx, by, top))
    return caps


# ---------------------------------------------------------------- comprobaciones
def check_fit(v, front, back):
    case = front + back
    out = outer(v)
    problems = []
    for name, part in components().items():
        hit = (part ^ case).volume()
        if name.endswith("USB-C"):
            continue  # los conectores asoman por su hueco a propósito
        if hit > 0.05:
            problems.append(f"{name}: choca {hit:.2f} mm³")
        outside = (part - out).volume()
        if outside > 0.05:
            problems.append(f"{name}: se sale {outside:.2f} mm³")
    return problems


def to_mesh(m):
    mesh = m.to_mesh()
    return np.asarray(mesh.vert_properties)[:, :3].astype(float), np.asarray(mesh.tri_verts).astype(np.int64)


def export_stl(m, path):
    import trimesh
    v, f = to_mesh(m)
    trimesh.Trimesh(v, f, process=False).export(path)


def main(out_dir):
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    ok = True
    for v in VARIANTS:
        front, back, caps = build(v)
        problems = check_fit(v, front, back)
        bb = (front + back).bounding_box()
        size = (bb[3] - bb[0], bb[4] - bb[1], bb[5] - bb[2])
        print(f"{v.name}: {size[0]:.1f} x {size[1]:.1f} x {size[2]:.1f} mm "
              f"(frente {front.volume() / 1000:.1f} cm³, tapa {back.volume() / 1000:.1f} cm³)")
        for p in problems:
            print("   CHOQUE:", p)
            ok = False
        export_stl(front.rotate((180, 0, 0)), out_dir / f"{v.name}_frente.stl")   # frontal boca abajo
        export_stl(back, out_dir / f"{v.name}_tapa.stl")                          # tapa apoyada en su espalda
        export_stl(M.batch_boolean(caps, __import__("manifold3d").OpType.Add), out_dir / f"{v.name}_botones.stl")
    return ok


if __name__ == "__main__":
    sys.exit(0 if main(sys.argv[1] if len(sys.argv) > 1 else "stl") else 1)

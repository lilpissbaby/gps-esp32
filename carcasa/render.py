"""Render sencillo por software (z-buffer + sombreado) para ver las carcasas sin programas 3D."""
import numpy as np
from PIL import Image, ImageDraw, ImageFont


def _basis(direction, up=(0, 1, 0)):
    d = np.asarray(direction, float)
    f = -d / np.linalg.norm(d)                 # la cámara mira hacia -direction
    r = np.cross(f, up)
    r /= np.linalg.norm(r)
    u = np.cross(r, f)
    return f, r, u


def render(parts, direction, size=900, ss=2, bg=(246, 245, 241), bounds=None, margin=0.08,
           outline=True):
    """parts: lista de (vertices Nx3, caras Mx3, color RGB 0-255). Proyección ortográfica."""
    f, r, u = _basis(direction)
    S = size * ss
    allv = np.vstack([p[0] for p in parts])
    X, Y = allv @ r, allv @ u
    if bounds is None:
        bounds = (X.min(), X.max(), Y.min(), Y.max())
    x0, x1, y0, y1 = bounds
    span = max(x1 - x0, y1 - y0) * (1 + 2 * margin)
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    scale = S / span

    zbuf = np.full((S, S), np.inf)
    img = np.zeros((S, S, 3))
    img[:] = np.array(bg) / 255.0
    nbuf = np.zeros((S, S, 3))
    light1 = -np.array([-0.4, -0.8, -0.6]); light1 /= np.linalg.norm(light1)
    light2 = -np.array([0.7, -0.2, 0.3]); light2 /= np.linalg.norm(light2)

    for verts, faces, color in parts:
        col = np.array(color) / 255.0
        px = (verts @ r - cx) * scale + S / 2
        py = S / 2 - (verts @ u - cy) * scale
        pz = verts @ f
        tri = verts[faces]
        n = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
        nl = np.linalg.norm(n, axis=1)
        keep = nl > 1e-12
        n[keep] /= nl[keep, None]
        visible = (n @ f) < 0
        shade = 0.38 + 0.5 * np.clip(n @ light1, 0, 1) + 0.22 * np.clip(n @ light2, 0, 1)
        for t in np.nonzero(visible & keep)[0]:
            i0, i1, i2 = faces[t]
            xs = np.array([px[i0], px[i1], px[i2]]); ys = np.array([py[i0], py[i1], py[i2]])
            zs = np.array([pz[i0], pz[i1], pz[i2]])
            xmin, xmax = max(int(xs.min()), 0), min(int(xs.max()) + 1, S - 1)
            ymin, ymax = max(int(ys.min()), 0), min(int(ys.max()) + 1, S - 1)
            if xmax < xmin or ymax < ymin:
                continue
            gx, gy = np.meshgrid(np.arange(xmin, xmax + 1) + 0.5, np.arange(ymin, ymax + 1) + 0.5)
            d = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2])
            if abs(d) < 1e-12:
                continue
            w0 = ((ys[1] - ys[2]) * (gx - xs[2]) + (xs[2] - xs[1]) * (gy - ys[2])) / d
            w1 = ((ys[2] - ys[0]) * (gx - xs[2]) + (xs[0] - xs[2]) * (gy - ys[2])) / d
            w2 = 1 - w0 - w1
            inside = (w0 >= -1e-6) & (w1 >= -1e-6) & (w2 >= -1e-6)
            if not inside.any():
                continue
            z = w0 * zs[0] + w1 * zs[1] + w2 * zs[2]
            sub = zbuf[ymin:ymax + 1, xmin:xmax + 1]
            upd = inside & (z < sub)
            if not upd.any():
                continue
            sub[upd] = z[upd]
            c = np.clip(col * shade[t], 0, 1)
            img[ymin:ymax + 1, xmin:xmax + 1][upd] = c
            nbuf[ymin:ymax + 1, xmin:xmax + 1][upd] = n[t]

    if outline:
        hit = np.isfinite(zbuf)
        zf = np.where(hit, zbuf, zbuf[hit].max() + 50 if hit.any() else 0)
        dz = np.zeros_like(zf)
        dz[1:-1, 1:-1] = np.maximum.reduce([abs(zf[1:-1, 2:] - zf[1:-1, :-2]),
                                            abs(zf[2:, 1:-1] - zf[:-2, 1:-1])])
        dn = np.zeros(zf.shape)
        dn[1:-1, 1:-1] = np.maximum(np.linalg.norm(nbuf[1:-1, 2:] - nbuf[1:-1, :-2], axis=2),
                                    np.linalg.norm(nbuf[2:, 1:-1] - nbuf[:-2, 1:-1], axis=2))
        edge = (dz > 1.2 / scale * ss * 3) | (dn > 0.9)
        img[edge] = img[edge] * 0.35
    im = Image.fromarray((np.clip(img, 0, 1) * 255).astype(np.uint8))
    return im.resize((size, size), Image.LANCZOS)


def label(im, text, sub=None):
    d = ImageDraw.Draw(im)
    try:
        font = ImageFont.truetype("DejaVuSans-Bold.ttf", 30)
        small = ImageFont.truetype("DejaVuSans.ttf", 22)
    except OSError:
        font = small = ImageFont.load_default()
    d.text((24, 18), text, fill=(40, 40, 40), font=font)
    if sub:
        d.text((24, 58), sub, fill=(90, 90, 90), font=small)
    return im

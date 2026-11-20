# Carcasas del GPS — conceptos v0

Tres variantes con las medidas reales de las piezas y el GPS M10 (28 × 28 × 8 mm) tumbado arriba. Con el NEO-6M serían unos 3 mm más finas.

| Variante | Medidas sin el asa | Idea |
| --- | --- | --- |
| Compacta | 44 × 117 × 32 mm | Lo justo alrededor de las piezas, cantos suaves |
| Robusta | 48 × 120 × 37 mm | Esquinas de parachoques, marco elevado que protege la pantalla, guarda en los botones, estrías de agarre, raíles en la espalda y hueco para un cristal de metacrilato de 1,2 mm |
| Ergonómica | 44 × 118 × 35 mm | Espalda abombada que llena la palma, cantos traseros muy redondeados y tres surcos para los dedos |

Cada variante tiene tres archivos en `stl/`:
- `*_frente.stl`: el frontal, ya girado para imprimirlo boca abajo.
- `*_tapa.stl`: la tapa trasera, apoyada en su espalda.
- `*_botones.stl`: los 4 capuchones (+, −, OK, ←), de pie.

## Imprimir

- **Material:** PETG, capas de 0,2 mm, 3 perímetros y 20 % de relleno.
- **Soportes:** la tapa de la ergonómica (espalda curva) y el frontal de la robusta (el marco y la guarda sobresalen) necesitan soportes.
- **Tornillos:** 4 autorroscantes M2,5 × 16 desde la tapa.
- **Placa perforada:** hay que recortar sus dos esquinas inferiores (2 × 2 agujeros) para dejar sitio a los tetones de los tornillos.

## Ojo: es un concepto

El alto de la placa de la pantalla (≈ 63 mm) está estimado por foto. Antes de la versión definitiva hay que medirlo e imprimir una prueba para comprobar el encaje.

## Regenerar

```bash
pip install manifold3d trimesh pillow numpy
python carcasa.py stl     # STL y comprobación de choques
python vistas.py img      # imágenes
```

Las medidas están al principio de `carcasa.py`.

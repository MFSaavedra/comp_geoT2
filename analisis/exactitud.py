#!/usr/bin/env python3
"""Compara las cerraduras calculadas en double con la cerradura exacta.

Uso, desde la raiz (despues de correr build/exactitud, ver exactitud.cpp):
    python3 analisis/exactitud.py build/exactitud_datos

Cada double es un racional exacto: con fractions.Fraction la orientacion no
tiene redondeo, y el algoritmo incremental da la cerradura verdadera de los
mismos puntos. Escribe la tabla del informe en analisis/exactitud.tex.
"""

import pathlib
import sys
from fractions import Fraction

SALIDA_TEX = pathlib.Path(__file__).resolve().parent / "exactitud.tex"


def orientacion(a, b, c):
    """Signo exacto de (b - a) x (c - a)."""
    v = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])
    return (v > 0) - (v < 0)


def cadena(puntos, exactos):
    """Una pasada del incremental: solo deja vueltas a la derecha."""
    c = []
    for q in puntos:
        while len(c) >= 2 and orientacion(exactos[c[-2]], exactos[c[-1]], exactos[q]) >= 0:
            c.pop()
        c.append(q)
    return c


def cerradura_exacta(puntos):
    """Incremental (upper y luego lower hull) con aritmetica racional, en CCW."""
    p = sorted(set(puntos))
    exactos = {q: (Fraction(q[0]), Fraction(q[1])) for q in p}
    if len(p) < 3:
        return p
    cw = cadena(p, exactos)[:-1] + cadena(p[::-1], exactos)[:-1]
    ccw = cw[::-1]
    i = ccw.index(min(ccw))
    return ccw[i:] + ccw[:i]


def leer(ruta):
    """Nube y cerraduras de un archivo de exactitud.cpp."""
    datos = {"P": [], "I": [], "R": []}
    for linea in ruta.read_text().splitlines():
        letra, x, y = linea.split()
        datos[letra].append((float.fromhex(x), float.fromhex(y)))
    return datos


def main():
    # Ejemplo con respuesta conocida (puntos interiores, colineales y repetidos).
    ejemplo = [(2, 2), (4, 4), (0, 0), (4, 0), (1, 3), (0, 4), (2, 0), (4, 2), (0, 0)]
    assert cerradura_exacta(ejemplo) == [(0, 0), (4, 0), (4, 4), (0, 4)]
    directorio = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".")
    resumen = {}
    for ruta in sorted(directorio.glob("*.txt")):
        n = int(ruta.stem.split("_")[0])
        d = leer(ruta)
        exacta = cerradura_exacta(d["P"])
        fila = resumen.setdefault(n, {"casos": 0, "h": [], "I": [0, [], []], "R": [0, [], []]})
        fila["casos"] += 1
        fila["h"].append(len(exacta))
        for alg in ("I", "R"):
            obtenida = d[alg]
            fila[alg][0] += obtenida == exacta
            fila[alg][1].append(len(set(exacta) - set(obtenida)))  # vertices que faltan
            fila[alg][2].append(len(set(obtenida) - set(exacta)))  # vertices de mas
    lineas = ["% generado por analisis/exactitud.py; no editar a mano", "\\newcommand{\\tablaExactitud}{%"]
    for n, f in sorted(resumen.items()):
        celdas = [f"{n:,}".replace(",", "\\,"), f"{min(f['h'])}--{max(f['h'])}"]
        for alg in ("I", "R"):
            ok, faltan, sobran = f[alg]
            celdas += [f"{ok}/{f['casos']}", f"{sum(faltan) / len(faltan):.1f}",
                       f"{sum(sobran) / len(sobran):.1f}"]
        lineas.append(" & ".join(celdas) + " \\\\")
        print(n, celdas)
    lineas.append("}")
    SALIDA_TEX.write_text("\n".join(lineas) + "\n")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Figuras y tablas de ANALISIS.pdf a partir de resultados/datos.csv.

Uso, desde la raiz del repositorio:  python3 analisis/graficos.py

Escribe las figuras en analisis/figuras/ y los numeros y tablas que cita el
informe en analisis/resultados.tex, para que el texto no copie valores a mano.
"""

import math
import pathlib

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker
import numpy as np
import pandas as pd

RAIZ = pathlib.Path(__file__).resolve().parent.parent
DATOS = RAIZ / "resultados" / "datos.csv"
FIGURAS = RAIZ / "analisis" / "figuras"
SALIDA_TEX = RAIZ / "analisis" / "resultados.tex"

INC, RAN = "incremental", "random_incremental"
NOMBRE = {INC: "Incremental", RAN: "Random Incremental"}
COLOR = {INC: "#1f6fb4", RAN: "#c8392b"}
LINEA = {"double": "-", "int": "--"}
MARCA = {"double": "o", "int": "s"}

plt.rcParams.update({"font.size": 9, "axes.grid": True, "grid.alpha": 0.3,
                     "figure.dpi": 150, "savefig.bbox": "tight"})


def mediana(df, columnas):
    """Mediana y cuartiles del tiempo, agrupando por las columnas dadas."""
    g = df.groupby(columnas)["tiempo_ms"]
    return pd.DataFrame({"med": g.median(), "q1": g.quantile(0.25),
                         "q3": g.quantile(0.75)}).reset_index()


def serie(ax, x, m, alg, tipo, etiqueta=True):
    """Dibuja una curva de medianas con su banda intercuartil."""
    ax.plot(x, m["med"], LINEA[tipo], marker=MARCA[tipo], ms=3, color=COLOR[alg],
            label=f"{NOMBRE[alg]}, {tipo}" if etiqueta else None)
    ax.fill_between(x, m["q1"], m["q3"], color=COLOR[alg], alpha=0.15, lw=0)


def numero(x, decimales=0):
    """Numero con separador de miles para el texto en LaTeX."""
    return f"{x:,.{decimales}f}".replace(",", "\\,")


def figura_escalamiento(d):
    """Tiempo vs n y tiempo / (n log2 n) vs n, sin y con todos en la cerradura."""
    e = d[d.experimento == "escalamiento"]
    m = mediana(e, ["generador", "tipo", "algoritmo", "n"])
    titulos = {"uniforme": "uniforme en el cuadrado",
               "circunferencia": "todos en la cerradura (circunferencia 100%)"}
    for normalizado in (False, True):
        fig, ejes = plt.subplots(1, 2, figsize=(7.2, 2.9), sharey=not normalizado)
        for ax, gen in zip(ejes, ["uniforme", "circunferencia"]):
            for alg in (INC, RAN):
                for tipo in ("double", "int"):
                    s = m[(m.generador == gen) & (m.algoritmo == alg) & (m.tipo == tipo)]
                    s = s.sort_values("n").copy()
                    if normalizado:
                        escala = 1e6 / (s["n"] * np.log2(s["n"]))  # ms -> ns por n log2 n
                        for c in ("med", "q1", "q3"):
                            s[c] = s[c] * escala
                    serie(ax, s["n"], s, alg, tipo)
            ax.set_xscale("log")
            if not normalizado:
                ax.set_yscale("log")
            ax.set_title(titulos[gen])
            ax.set_xlabel("n")
        ejes[0].set_ylabel("ns / (n log$_2$ n)" if normalizado else "tiempo (ms)")
        ejes[0].legend(fontsize=7)
        nombre = "escalamiento_normalizado" if normalizado else "escalamiento"
        fig.savefig(FIGURAS / f"{nombre}.pdf")
        plt.close(fig)
    return m


def figura_porcentaje(d):
    """Tiempo, y cociente de tiempos, vs porcentaje en la cerradura; un panel por n."""
    e = d[d.experimento == "porcentaje"]
    m = mediana(e, ["tipo", "algoritmo", "n", "parametro"])
    tamanos = sorted(e["n"].unique())
    fig, ejes = plt.subplots(2, len(tamanos), figsize=(7.2, 4.8), sharex=True)
    for col, n in enumerate(tamanos):
        arriba, abajo = ejes[0][col], ejes[1][col]
        for tipo in ("double", "int"):
            for alg in (INC, RAN):
                s = m[(m.n == n) & (m.algoritmo == alg) & (m.tipo == tipo)]
                s = s.sort_values("parametro")
                serie(arriba, 100 * s["parametro"], s, alg, tipo)
            c = m[(m.n == n) & (m.tipo == tipo)].pivot(index="parametro", columns="algoritmo",
                                                         values="med")
            abajo.plot(100 * c.index, c[RAN] / c[INC], LINEA[tipo], marker=MARCA[tipo], ms=3,
                       color="#555555", label=tipo)
        abajo.axhline(1, color="black", lw=0.8)
        for ax, marcas in ((arriba, (1.0, 2.0, 5.0)), (abajo, (1.0, 1.5, 2.0, 3.0, 5.0))):
            ax.set_yscale("log")
            ax.yaxis.set_major_locator(matplotlib.ticker.LogLocator(base=10, subs=marcas))
            ax.yaxis.set_major_formatter(matplotlib.ticker.FormatStrFormatter("%g"))
            ax.yaxis.set_minor_formatter(matplotlib.ticker.NullFormatter())
        abajo.set_xscale("log")
        abajo.xaxis.set_major_formatter(matplotlib.ticker.FormatStrFormatter("%g"))
        arriba.set_title(f"n = {numero(n)}".replace("\\,", " "))
        abajo.set_xlabel("% de puntos en la cerradura")
    ejes[0][0].set_ylabel("tiempo (ms)")
    ejes[1][0].set_ylabel("t Random Incr. / t Incremental")
    ejes[0][0].legend(fontsize=6)
    ejes[1][0].legend(fontsize=7)
    fig.savefig(FIGURAS / "porcentaje.pdf")
    plt.close(fig)
    return m


def figura_reasignaciones(d):
    """Reasignaciones / n del aleatorio y tamano real de la cerradura con int."""
    e = d[(d.experimento == "porcentaje") & (d.algoritmo == RAN)]
    fig, (a, b) = plt.subplots(1, 2, figsize=(7.2, 2.7))
    for n, color in zip(sorted(e["n"].unique()), ["#7a9cc6", "#3c6ea8", "#173f6e"]):
        for tipo in ("double", "int"):
            s = e[(e.n == n) & (e.tipo == tipo)].groupby("parametro").median(numeric_only=True)
            etiqueta = f"n = {numero(n)}, {tipo}".replace("\\,", " ")
            a.plot(100 * s.index, s["reasignaciones"] / n, LINEA[tipo], marker=MARCA[tipo],
                   ms=3, color=color, label=etiqueta)
            if tipo == "int":
                pedido = np.maximum(3, np.round(s.index * n))
                b.plot(100 * s.index, s["h"] / pedido, "--", marker="s", ms=3, color=color,
                       label=f"n = {numero(n)}".replace("\\,", " "))
    for n, color in zip(sorted(e["n"].unique()), ["#7a9cc6", "#3c6ea8", "#173f6e"]):
        a.axhline(2 * math.log(n), color=color, lw=0.8, ls=":")  # cota 2 H_n ~ 2 ln n
    a.plot([], [], ":", color="gray", lw=0.8, label="cota 2 ln n")
    a.set_xscale("log")
    a.set_xlabel("% de puntos en la cerradura")
    a.set_ylabel("reasignaciones / n")
    a.set_title("trabajo del Random Incremental")
    a.legend(fontsize=6)
    b.set_xscale("log")
    b.set_xlabel("% pedido de puntos en la cerradura")
    b.set_ylabel("h obtenido / h pedido")
    b.set_title("coordenadas int: el redondeo quita vertices")
    b.legend(fontsize=7)
    fig.savefig(FIGURAS / "reasignaciones.pdf")
    plt.close(fig)


def cruce(m):
    """Porcentaje en que el aleatorio deja de ser mas rapido, por tipo y n.

    Interpola log(t_ran / t_inc) contra log(p) entre los dos porcentajes
    medidos donde el cociente cruza 1. None si no cruza.
    """
    filas = []
    for tipo in ("double", "int"):
        for n in sorted(m["n"].unique()):
            s = m[(m.tipo == tipo) & (m.n == n)].pivot(index="parametro", columns="algoritmo",
                                                         values="med")
            r = np.log(s[RAN] / s[INC])
            p = np.log(s.index.to_numpy())
            punto = None
            for i in range(len(r) - 1):
                if r.iloc[i] < 0 <= r.iloc[i + 1]:
                    t = -r.iloc[i] / (r.iloc[i + 1] - r.iloc[i])
                    punto = math.exp(p[i] + t * (p[i + 1] - p[i]))
                    break
            filas.append((tipo, n, punto, s[RAN].iloc[0] / s[INC].iloc[0],
                          s[RAN].iloc[-1] / s[INC].iloc[-1]))
    return filas


def descomposicion(d):
    """Descompone el cociente de tiempos con el menor porcentaje medido.

    t_inc = c_inc * n log2 n  y  t_ran = c_ran * R, con R las reasignaciones:
    t_ran / t_inc = [(R / n) / log2 n] * [c_ran / c_inc], un cociente de
    cantidad de operaciones por un cociente de costo por operacion.
    """
    e = d[d.experimento == "porcentaje"]
    filas = []
    for tipo in ("double", "int"):
        for n in sorted(e["n"].unique()):
            s = e[(e.tipo == tipo) & (e.n == n) & (e.parametro == e.parametro.min())]
            inc = s[s.algoritmo == INC]["tiempo_ms"].median()
            ran = s[s.algoritmo == RAN]
            r = ran["reasignaciones"].median()
            c_inc = 1e6 * inc / (n * math.log2(n))
            c_ran = 1e6 * ran["tiempo_ms"].median() / r
            operaciones = (r / n) / math.log2(n)
            filas.append((tipo, n, math.log2(n), r / n, c_inc, c_ran, operaciones,
                          c_ran / c_inc, ran["tiempo_ms"].median() / inc))
    return filas


def costo_reasignacion(d):
    """Rango de ns por reasignacion del aleatorio, por tipo y n (todos los porcentajes)."""
    e = d[(d.experimento == "porcentaje") & (d.algoritmo == RAN)]
    filas = {}
    for (tipo, n), s in e.groupby(["tipo", "n"]):
        g = s.groupby("parametro")[["tiempo_ms", "reasignaciones"]].median()
        c = 1e6 * g["tiempo_ms"] / g["reasignaciones"]
        filas[(tipo, n)] = (c.min(), c.max())
    return filas


def tabla_varianza(d):
    """Media, desviacion y coeficiente de variacion con 20 semillas."""
    e = d[d.experimento == "varianza"]
    filas = []
    for tipo in ("double", "int"):
        for gen in ("uniforme", "circunferencia"):
            for alg in (INC, RAN):
                s = e[(e.tipo == tipo) & (e.generador == gen) & (e.algoritmo == alg)]
                t, r = s["tiempo_ms"], s["reasignaciones"]
                filas.append((tipo, gen, alg, t.mean(), t.std(), 100 * t.std() / t.mean(),
                              r.mean() if alg == RAN else None,
                              100 * r.std() / r.mean() if alg == RAN else None))
    return filas


def tabla_colinealidad(d):
    """Coincidencia y tamano de las cerraduras con y sin puntos colineales."""
    e = d[d.experimento == "colinealidad"]
    filas = []
    for gen in ("cuadrado_0", "cuadrado_30", "circunferencia"):
        for tipo in ("int", "double"):
            for n in sorted(e["n"].unique()):
                s = e[(e.generador == gen) & (e.tipo == tipo) & (e.n == n)]
                i, r = s[s.algoritmo == INC], s[s.algoritmo == RAN]
                filas.append((gen, tipo, n, 100 * i["iguales"].mean(), i["h"].median(),
                              r["h"].median(), i["h"].min(), i["h"].max(), r["h"].min(),
                              r["h"].max()))
    return filas


def escribir_tex(d, m_esc, filas_cruce, filas_ajuste, filas_var, filas_col):
    """Macros y tablas para ANALISIS.tex."""
    l = ["% generado por analisis/graficos.py; no editar a mano"]

    def macro(nombre, valor):
        l.append(f"\\newcommand{{\\{nombre}}}{{{valor}}}")

    pares = d[d.algoritmo == INC]
    macro("resCasos", numero(len(pares)))
    macro("resIguales", numero(int(pares["iguales"].sum())))
    for exp, nombre in (("escalamiento", "Esc"), ("porcentaje", "Por"),
                        ("colinealidad", "Col"), ("varianza", "Var")):
        s = pares[pares.experimento == exp]
        macro(f"resIguales{nombre}", f"{int(s['iguales'].sum())}/{len(s)}")

    # Tiempos a n = 10^6 del escalamiento, y cocientes.
    for gen, g in (("uniforme", "Uni"), ("circunferencia", "Cir")):
        for tipo, t in (("double", "D"), ("int", "I")):
            for alg, a in ((INC, "Inc"), (RAN, "Ran")):
                v = m_esc[(m_esc.generador == gen) & (m_esc.tipo == tipo) &
                          (m_esc.algoritmo == alg) & (m_esc.n == 10**6)]["med"].iloc[0]
                macro(f"res{g}{t}{a}", numero(v))
            inc = m_esc[(m_esc.generador == gen) & (m_esc.tipo == tipo) &
                        (m_esc.algoritmo == INC) & (m_esc.n == 10**6)]["med"].iloc[0]
            ran = m_esc[(m_esc.generador == gen) & (m_esc.tipo == tipo) &
                        (m_esc.algoritmo == RAN) & (m_esc.n == 10**6)]["med"].iloc[0]
            macro(f"res{g}{t}Cociente", numero(ran / inc, 2))
    for alg, a in ((INC, "Inc"), (RAN, "Ran")):
        for gen, g in (("uniforme", "Uni"), ("circunferencia", "Cir")):
            sel = (m_esc.generador == gen) & (m_esc.algoritmo == alg) & (m_esc.n == 10**6)
            di = m_esc[sel & (m_esc.tipo == "int")]["med"].iloc[0]
            dd = m_esc[sel & (m_esc.tipo == "double")]["med"].iloc[0]
            macro(f"resIntDouble{g}{a}", numero(di / dd, 2))
    # Rango de ns / (n log2 n) del Incremental uniforme en double.
    s = m_esc[(m_esc.generador == "uniforme") & (m_esc.tipo == "double") & (m_esc.algoritmo == INC)]
    norm = 1e6 * s["med"] / (s["n"] * np.log2(s["n"]))
    macro("resIncNormMin", numero(norm.min(), 1))
    macro("resIncNormMax", numero(norm.max(), 1))
    # Random Incremental con todos en la cerradura: ns / (n log2 n) en 10^4 y 10^6.
    s = m_esc[(m_esc.generador == "circunferencia") & (m_esc.tipo == "double") &
              (m_esc.algoritmo == RAN)].set_index("n")["med"]
    for n, nombre in ((10**4, "Min"), (10**6, "Max")):
        macro(f"resCirRanNorm{nombre}", numero(1e6 * s[n] / (n * math.log2(n)), 0))
    # Costo por reasignacion (ns), rango sobre los porcentajes.
    costos = costo_reasignacion(d)
    pequenos = [c for (tipo, n), par in costos.items() if n < 10**6 for c in par]
    macro("resCostoPequeMin", numero(min(pequenos), 0))
    macro("resCostoPequeMax", numero(max(pequenos), 0))
    grandes = [c for (tipo, n), par in costos.items() if n == 10**6 for c in par]
    macro("resCostoGrandeMin", numero(min(grandes), 0))
    macro("resCostoGrandeMax", numero(max(grandes), 0))
    # Reasignaciones por punto con el menor porcentaje.
    e = d[(d.experimento == "porcentaje") & (d.algoritmo == RAN)]
    e = e[e.parametro == e.parametro.min()]
    rn = (e["reasignaciones"] / e["n"]).groupby([e["tipo"], e["n"]]).median()
    macro("resRporNMin", numero(rn.min(), 1))
    macro("resRporNMax", numero(rn.max(), 1))
    # Ruido de la maquina: la misma nube con la misma semilla, al inicio
    # (escalamiento, instancia 0) y al final (varianza, semilla 1) de cada tipo.
    dif = []
    for tipo in ("double", "int"):
        for gen in ("uniforme", "circunferencia"):
            for alg in (INC, RAN):
                a = d[(d.experimento == "escalamiento") & (d.tipo == tipo) & (d.generador == gen) &
                      (d.n == 10**6) & (d.instancia == 0) & (d.algoritmo == alg)]["tiempo_ms"].iloc[0]
                b = d[(d.experimento == "varianza") & (d.tipo == tipo) & (d.generador == gen) &
                      (d.semilla == 1) & (d.algoritmo == alg)]["tiempo_ms"].iloc[0]
                dif.append(100 * abs(a - b) / min(a, b))
    macro("resRuidoMin", numero(min(dif), 0))
    macro("resRuidoMax", numero(max(dif), 0))
    # Correlacion entre tiempo y reasignaciones con 20 semillas.
    for tipo, t in (("double", "D"), ("int", "I")):
        for gen, g in (("uniforme", "Uni"), ("circunferencia", "Cir")):
            v = d[(d.experimento == "varianza") & (d.tipo == tipo) & (d.generador == gen) &
                  (d.algoritmo == RAN)]
            macro(f"resCorr{g}{t}", numero(v["tiempo_ms"].corr(v["reasignaciones"]), 2))
    # Puntos uniformes: el cociente mas favorable al aleatorio, y en que n.
    for tipo, t in (("double", "D"), ("int", "I")):
        u = m_esc[(m_esc.generador == "uniforme") & (m_esc.tipo == tipo)]
        c = u.pivot(index="n", columns="algoritmo", values="med")
        c = c[RAN] / c[INC]
        macro(f"resUniCocMin{t}", numero(c.min(), 2))
        macro(f"resUniCocMinN{t}", numero(c.idxmin()))
    # Hull de los uniformes a 10^6.
    h = d[(d.experimento == "escalamiento") & (d.generador == "uniforme") & (d.n == 10**6) &
          (d.algoritmo == INC) & (d.tipo == "double")]["h"]
    macro("resUniH", numero(h.median()))
    macro("resUniHEsperado", numero(8 / 3 * math.log(10**6)))
    # Reasignaciones del aleatorio con todos en la cerradura a 10^6.
    r = d[(d.experimento == "escalamiento") & (d.generador == "circunferencia") & (d.n == 10**6) &
          (d.algoritmo == RAN) & (d.tipo == "double")]["reasignaciones"]
    macro("resReasigTodos", numero(r.median()))
    macro("resReasigTodosCota", numero(2 * 10**6 * sum(1 / i for i in range(1, 10**6 + 1))))
    macro("resReasigTodosPorN", numero(r.median() / 10**6, 1))
    # Tamano real con int y 100% a 10^6.
    hi = d[(d.experimento == "porcentaje") & (d.tipo == "int") & (d.parametro == 1) &
           (d.n == 10**6) & (d.algoritmo == INC)]["h"]
    macro("resIntTodosH", numero(hi.median()))
    hi5 = d[(d.experimento == "porcentaje") & (d.tipo == "int") & (d.n == 10**5) &
            (d.parametro == 1) & (d.algoritmo == INC)]["h"]
    macro("resIntTodosHcienmil", numero(hi5.median()))

    l.append("\\newcommand{\\tablaCruce}{%")
    for tipo, n, p, r0, r1 in filas_cruce:
        celda = "no se cruzan" if p is None else f"{100 * p:.2g}\\,\\%"
        l.append(f"{tipo} & {numero(n)} & {r0:.2f} & {r1:.2f} & {celda} \\\\")
    l.append("}")

    l.append("\\newcommand{\\tablaDescomposicion}{%")
    for tipo, n, lg, rn, ci, cr, ops, costos, medido in filas_ajuste:
        l.append(f"{tipo} & {numero(n)} & {lg:.1f} & {rn:.2f} & {ops:.2f} & {ci:.1f} & {cr:.1f} & "
                 f"{costos:.1f} & {medido:.2f} \\\\")
    l.append("}")

    l.append("\\newcommand{\\tablaVarianza}{%")
    nombres_gen = {"uniforme": "uniforme", "circunferencia": "100\\,\\% en la cerradura"}
    for tipo, gen, alg, media, desv, cv, rm, rcv in filas_var:
        reas = "-- & --" if rm is None else f"{numero(rm)} & {rcv:.2f}\\,\\%"
        l.append(f"{tipo} & {nombres_gen[gen]} & {NOMBRE[alg]} & {media:.1f} & {desv:.1f} & "
                 f"{cv:.1f}\\,\\% & {reas} \\\\")
    l.append("}")

    l.append("\\newcommand{\\tablaColinealidad}{%")
    nombres_col = {"cuadrado_0": "cuadrado, 0\\textdegree", "cuadrado_30": "cuadrado, 30\\textdegree",
                   "circunferencia": "circunferencia"}
    anterior = None
    for gen, tipo, n, ig, hi_, hr, himin, himax, hrmin, hrmax in filas_col:
        if anterior is not None and gen != anterior:
            l.append("\\midrule")
        anterior = gen
        rango_i = numero(himin) if himin == himax else f"{numero(himin)}--{numero(himax)}"
        rango_r = numero(hrmin) if hrmin == hrmax else f"{numero(hrmin)}--{numero(hrmax)}"
        l.append(f"{nombres_col[gen]} & {tipo} & {numero(n)} & {ig:.0f}\\,\\% & {rango_i} & "
                 f"{rango_r} \\\\")
    l.append("}")

    SALIDA_TEX.write_text("\n".join(l) + "\n")


def main():
    FIGURAS.mkdir(parents=True, exist_ok=True)
    d = pd.read_csv(DATOS)
    m_esc = figura_escalamiento(d)
    m_por = figura_porcentaje(d)
    figura_reasignaciones(d)
    escribir_tex(d, m_esc, cruce(m_por), descomposicion(d), tabla_varianza(d), tabla_colinealidad(d))
    print(f"figuras en {FIGURAS}, numeros en {SALIDA_TEX}")


if __name__ == "__main__":
    main()

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Scope of the tree

`/home/lynx/box/dcc/comp_geometry` is a course workspace for CC5502 Geometría Computacional (DCC), **not** a git repository. This directory, `comp_geoT2/`, is the Tarea 2 deliverable (`origin` = `git@github.com:MFSaavedra/comp_geoT2.git`, private); it started as a copy of the full history of `comp_geoT1/` (Tarea 1, public repo `MFSaavedra/comp_geoT1`), because the spec requires building Tarea 2 on top of Tarea 1. Run `git` from here. The spec is `../tarea2_CH.pdf`. Everything else in the workspace is read-only reference material: `libro/` (de Berg, O'Rourke), `Lecturas/`, `Lectura_2.pdf` (Moret, experimental algorithmics — the experiment's methodology), and the professor's material.

Course material and the assignment are in Spanish; code identifiers, comments, Doxygen blocks, the README and `ANALISIS.tex` follow suit. Exceptions mandated by the spec: `IncrementalAlgorithm` and `apply`.

## What Tarea 2 asks for

Two convex hull algorithms behind a Strategy interface, point-set generators (also Strategy), a `main` that compares them, `ANALISIS.pdf` (assumptions, conditions, results, plots of time vs n, int vs double, collinear vs not), and a README. Both hulls must be **identical** for every input.

## Build, test, run

```bash
cmake -S . -B build && cmake --build build
cd build && ctest --output-on-failure        # Google Test, test/ (singular)
./build/main                                  # demo + quick comparison (~10 s)
taskset -c 3 ./build/experimento resultados/datos.csv   # full experiment (~8 min)
python3 analisis/graficos.py && (cd analisis && latexmk -pdf ANALISIS.tex && cp ANALISIS.pdf ..)
doxygen                                       # docs/html/, committed
```

- `CMakeLists.txt` defaults `CMAKE_BUILD_TYPE` to `Release`; never take timings from an unoptimized build.
- New `test/*.cpp` are picked up automatically (`CONFIGURE_DEPENDS`); `gtest_discover_tests` runs the test binary at build time, so a crash during static init fails the *build*.
- `geo` is an `INTERFACE` library: use `INTERFACE` on any `target_*` call against it.
- Worth re-running after touching `RandomIncrementalAlgorithm.hpp`: the test suite under `-fsanitize=address,undefined` (plain `g++ ... test/*.cpp -lgtest -lgtest_main`).

## Architecture

All templates, so header-only: `src/` holds only `main.cpp` and `experimento.cpp`.

- `Punto` / `Vector` / `Poligono` — Tarea 1. Additions for Tarea 2: `Punto::operator<` (exact lexicographic), `Poligono()`, `Poligono::normalizar()` (CCW + rotate to the smallest vertex), `Poligono::operator==` (vertex by vertex).
- `Geometria.hpp` — `orientacion(a,b,c)` returns the sign (-1/0/1) of the cross product computed in `Ancho<T>` **through `Vector::productoCruz`** (via `signoCruz`); `mismoPunto` is exact equality. Both algorithms use only these predicates. `Ancho<T>`: `long long` for integral T up to 32 bits, `Entero128` (`__int128`) for `long long`, `std::common_type_t<T, double>` for floating T (so `long double` stays `long double`). `esTipoCoordenada<T>` lists the accepted types — short, int, unsigned, long long, float, double, long double — and both Strategy bases `static_assert` on it. `Punto`/`Vector`/`Poligono` stay unconstrained: the predicates themselves instantiate `Vector<long long>` and `Vector<Entero128>`.
- `EstrategiaCerradura<T>` — `apply` (non-const: strategies may keep state such as the reassignment counter) and `nombre`. The output convention (extreme vertices only, normalized, degenerate cases) is documented there and is what makes `==` meaningful; keep both algorithms on it.
- `IncrementalAlgorithm` — sort, unique, upper hull left→right, lower hull right→left, pop on `orientacion >= 0`.
- `RandomIncrementalAlgorithm` — CMU notes §2.2. Fixed interior point C = centroid of the first triangle, handled as 3C (`sumaX/sumaY`) so integer math stays exact; each future point stores the edge its ray from C crosses (`arista`, edge = tail vertex), each edge a contiguous bucket (`std::vector<int>`). Strict outside test, non-strict walk (removes edges whose line passes through q). Points that fall inside are dropped at reassignment. `ladoDelRayo` calls `signoCruz` on vectors already in `Ancho<T>` (calling `orientacion` would promote them again, to `Entero128` for int). Integral T requires, per axis, max − min < 2^29 (≤ 32-bit T) or < 2^61 (`long long`): the 3C cross product is < 18 L².
- `GeneradorPuntos.hpp` — uniform square, circle (p·n on the circle, rest in the inscribed disk of the regular k-gon), rotated square boundary (collinear). Work in double in radius `RADIO` (1e8; 2e4 for short so the rotated corners fit), round for integral T and add `CENTRO` (141421357 for unsigned, else 0), so every type sees the same figure up to scale/translation. int and double output must stay bit-identical: the report's data depends on it.

Performance history (in `resultados/versiones_random_incremental.txt` and the report): intrusive bucket lists and an array-of-structs layout were both slower than the contiguous `std::vector<int>` buckets. Don't reintroduce them without measuring.

## Conventions

- Doxygen on every member: `@brief` always, `@param`/`@return`/`@tparam` where they apply, `@note` for code that looks wrong without an explanation. Short blocks.
- Metric results return `double`; promote before arithmetic that can overflow. Exact predicates promote to `Ancho<T>`, not `double`.
- `Punto::operator==` is tolerant for floating point; inside algorithms use the exact `mismoPunto`.
- Every public method has a test.
- Keep the code simple and readable — the user asked for it explicitly.

## Data and report

- `resultados/datos.csv` is the data the report was built from; `analisis/graficos.py` turns it into `analisis/figuras/*.pdf` and `analisis/resultados.tex` (macros/tables the report `\input`s — never hand-copy numbers into `ANALISIS.tex`).
- `.gitignore` excludes `*.pdf` except `ANALISIS.pdf` at the root (a required deliverable). Figures are build artifacts.
- `docs/html/` is committed; regenerate with `doxygen` after changing headers.

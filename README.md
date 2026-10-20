# comp_geoT2

Tarea 2 del curso CC5502 Geometría Computacional (DCC): cerradura convexa.
Implementa, sobre las clases `Punto`, `Vector` y `Poligono` de la Tarea 1, dos
algoritmos de cerradura convexa con el patrón Strategy, generadores de nubes de
puntos de prueba y un experimento que los compara. El análisis está en
`ANALISIS.pdf`.

- **Incremental** (`IncrementalAlgorithm`): ordena los puntos, construye el
  upper hull de izquierda a derecha y luego, devolviéndose, el lower hull.
  O(n log n).
- **Random Incremental** (`RandomIncrementalAlgorithm`): parte de un triángulo e
  inserta los demás puntos en orden aleatorio; cada punto que falta por
  insertar recuerda una arista que ve (sección 2 de las notas de CMU que cita
  el enunciado). O(n log n) esperado.

Ambos heredan de `EstrategiaCerradura` y sobreescriben
`Poligono<T> apply(const std::vector<Punto<T>> &nube)`. Los dos entregan la
cerradura de la misma forma, así que se comparan con `==`:

- solo los vértices extremos: los puntos colineales sobre una arista y los
  repetidos no aparecen;
- en sentido antihorario (CCW), desde el menor vértice en orden lexicográfico
  (`Poligono::normalizar`);
- con todos los puntos colineales, el segmento entre los dos extremos.

## Estructura

```
.
├── CMakeLists.txt
├── Doxyfile
├── ANALISIS.pdf                    # el informe (se compila desde analisis/)
├── analisis/
│   ├── ANALISIS.tex                # fuente del informe
│   ├── graficos.py                 # figuras y tablas desde resultados/datos.csv
│   ├── resultados.tex              # números y tablas generados por graficos.py
│   ├── exactitud.cpp, exactitud.py # verificación exacta del caso casi colineal
│   └── exactitud.tex               # tabla generada por exactitud.py
├── docs/html/                      # documentación generada con Doxygen
├── include/
│   ├── Punto.hpp, Vector.hpp, Poligono.hpp   # Tarea 1 (con algunos agregados)
│   ├── Geometria.hpp               # orientacion(), mismoPunto() y los tipos válidos
│   ├── EstrategiaCerradura.hpp     # interfaz Strategy de los algoritmos
│   ├── IncrementalAlgorithm.hpp
│   ├── RandomIncrementalAlgorithm.hpp
│   ├── GeneradorPuntos.hpp         # interfaz Strategy y los tres generadores
│   └── Medicion.hpp                # medirMs()
├── resultados/
│   ├── datos.csv                   # salida del experimento usada en el informe
│   └── versiones_random_incremental.txt   # tiempos de tres implementaciones
├── src/
│   ├── main.cpp                    # ejemplo y comparación rápida
│   └── experimento.cpp             # experimento completo -> CSV
└── test/                           # Google Test
```

Como todo son templates, el código vive en los headers de `include/`; `src/`
solo contiene los dos programas.

### Generadores

Todos heredan de `GeneradorPuntos` y trabajan en un disco de radio 10^8
(2·10^4 con `short`); con coordenadas enteras se redondea al entero más
cercano, y con `unsigned` la figura se traslada para que no haya coordenadas
negativas (ver la sección Supuestos).

| Generador | Nube |
|---|---|
| `GeneradorUniforme` | n puntos uniformes en un cuadrado |
| `GeneradorCircunferencia(p)` | p·n puntos sobre una circunferencia (la cerradura) y el resto uniforme dentro; sin colineales |
| `GeneradorCuadrado(p, ángulo)` | p·n puntos sobre el borde de un cuadrado rotado (con las 4 esquinas) y el resto dentro; con colineales |

## Requisitos

- CMake ≥ 3.14 y un compilador con soporte de C++17 (`g++` o `clang++`).
- Google Test, opcional: si está instalado (`libgtest-dev` en Debian/Ubuntu,
  `gtest` en Arch) se usa; si no, CMake lo descarga de GitHub al configurar.
- Para regenerar el informe: Python 3 con `pandas` y `matplotlib`, y LaTeX
  (`latexmk`, `pdflatex`).

## Compilar

Desde la raíz del repositorio:

```bash
cmake -S . -B build
cmake --build build
```

Genera `build/main`, `build/experimento` y `build/geo_tests`. Si no se indica
otro, el tipo de build es `Release` (con optimización): sin optimización los
tiempos medidos no son representativos.

## Ejecutar

```bash
./build/main
```

Calcula la cerradura de un ejemplo pequeño con ambas estrategias y luego, para
cada tipo de coordenadas y cada generador, mide las dos sobre la misma nube e
indica si las cerraduras son idénticas: `int` y `double` con n = 10^4, 10^5 y
10^6, y los demás tipos con n = 10^4 y 10^5 (unos 12 segundos). Termina con
código 0 si todas coinciden.

## Experimento completo

```bash
mkdir -p resultados
taskset -c 3 ./build/experimento resultados/datos.csv   # unos 8 minutos
```

`taskset` fija el proceso a un núcleo y es opcional (Linux). El CSV tiene una
fila por algoritmo y caso: experimento, tipo, generador, porcentaje, n,
instancia, semilla, algoritmo, tiempo (ms), h, reasignaciones y si las dos
cerraduras fueron idénticas. Las semillas son fijas: h, las reasignaciones y
las coincidencias se reproducen exactamente; los tiempos dependen de la
máquina.

## Regenerar el informe

```bash
python3 analisis/graficos.py
cd analisis && latexmk -pdf ANALISIS.tex && cp ANALISIS.pdf ..
```

`graficos.py` escribe las figuras en `analisis/figuras/` y los números que cita
el informe en `analisis/resultados.tex`.

### Verificación exacta del caso casi colineal

Para el cuadrado rotado con `double`, el informe compara ambas cerraduras con
la cerradura exacta de los mismos puntos, calculada con aritmética racional:

```bash
g++ -std=c++17 -O2 -Iinclude analisis/exactitud.cpp -o build/exactitud
mkdir -p build/exactitud_datos && ./build/exactitud build/exactitud_datos
python3 analisis/exactitud.py build/exactitud_datos   # ~1 minuto
```

Escribe la tabla en `analisis/exactitud.tex`.

## Ejecutar los tests

```bash
cd build && ctest --output-on-failure
```

o, para ver la salida completa de Google Test, `./build/geo_tests`.

## Sin CMake

Con Google Test instalado en el sistema:

```bash
g++ -std=c++17 -O3 -Iinclude src/main.cpp -o main && ./main
g++ -std=c++17 -O3 -Iinclude src/experimento.cpp -o experimento
g++ -std=c++17 -Iinclude test/*.cpp -lgtest -lgtest_main -pthread -o geo_tests && ./geo_tests
```

## Documentación

Los headers están comentados con Doxygen. Desde la raíz del repositorio:

```bash
doxygen
```

genera la documentación en `docs/html/index.html`.

## Supuestos

- Los algoritmos y los generadores aceptan coordenadas `short`, `int`,
  `unsigned`, `long long`, `float`, `double` y `long double`
  (`esTipoCoordenada`, en `Geometria.hpp`); un `static_assert` en
  `EstrategiaCerradura` y en `GeneradorPuntos` rechaza cualquier otro tipo al
  compilar. El producto cruz se calcula en `Ancho<T>`, que nunca es menos
  preciso que `T`:

  | Tipo | `Ancho<T>` | Coordenadas válidas | Generadores |
  |---|---|---|---|
  | `short` | `long long` | todo el rango del tipo | radio 2·10^4 |
  | `int` | `long long` | en cada eje, máx − mín < 2^29 (p. ej. ±2^28) | radio 10^8 |
  | `unsigned` | `long long` | en cada eje, máx − mín < 2^29 | radio 10^8, trasladado en 141 421 357 |
  | `long long` | `__int128` | en cada eje, máx − mín < 2^61 | radio 10^8 |
  | `float` | `double` | cualquiera, con redondeo | radio 10^8 |
  | `double` | `double` | cualquiera, con redondeo | radio 10^8 |
  | `long double` | `long double` | cualquiera, con redondeo | radio 10^8 |

- Con coordenadas enteras dentro de esos rangos los cálculos son exactos. El
  límite lo pone el Random Incremental, que trabaja con 3 veces el centroide:
  su producto cruz es menor que 18·(máx − mín)^2 y debe caber en `Ancho<T>`.
  `__int128` es una extensión de GCC y Clang (`Entero128` en el código).
- Los generadores producen la misma figura en todos los tipos, salvo escala y
  traslación: las esquinas del cuadrado rotado quedan a radio·√2 del centro,
  así que con radio 10^8 hay a lo más 2.83·10^8 < 2^29 en cada eje. Con
  `short` el radio es menor para que las coordenadas quepan, y el redondeo
  saca de la posición convexa a parte de los puntos de la circunferencia (de
  1000, quedan unos 800 en la cerradura). Con `unsigned` la traslación evita
  las coordenadas negativas, y los puntos son exactamente los de `int` más
  141 421 357. Con `float` las coordenadas se redondean con paso 8 cerca de
  10^8.
- Con tipos flotantes el producto cruz se redondea; con puntos casi colineales
  los algoritmos pueden no coincidir (ver `ANALISIS.pdf`, sección de
  colinealidad). El informe mide solo `int` y `double`.
- `Punto`, `Vector` y `Poligono` siguen siendo genéricos: los predicados los
  usan internamente con `long long` y `__int128`.

## Agregados a la Tarea 1

- `Punto::operator<`: orden lexicográfico exacto, para ordenar.
- `Poligono()`: polígono vacío; `Poligono::normalizar()`: forma canónica
  (CCW, desde el menor vértice); `Poligono::operator==` y `operator!=`.
- Con coordenadas `long double`, `Poligono` calcula el área con signo en
  `long double` y no en `double`, para que `hacerCCW` vea la misma precisión
  que los vértices.

## Declaración de uso de IA

Se utilizó Claude (Opus 5.5) para implementar el código, los tests y el
experimento, para discutir conceptos e ideas de diseño, y para redactar el
informe y la documentación.

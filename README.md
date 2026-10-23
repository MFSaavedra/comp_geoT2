# comp_geoT2

Tarea 2 del curso CC5502 Geometría Computacional (DCC): cerradura convexa.
Implementa, sobre las clases `Punto`, `Vector` y `Poligono` de la Tarea 1, dos
algoritmos de cerradura convexa con el patrón Strategy, generadores de nubes de
puntos de prueba y un experimento que los compara. El análisis está en
`ANALISIS.pdf`, y la documentación de las clases, generada con Doxygen, en
`docs/html/index.html`.

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
│   ├── Geometria.hpp               # orientacion() y mismoPunto()
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

Todos heredan de `GeneradorPuntos` y trabajan en un disco de radio 10^8;
con coordenadas `int` se redondea al entero más cercano.

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
cada generador, `int` y `double`, y n = 10^4, 10^5 y 10^6, mide las dos sobre la
misma nube e indica si las cerraduras son idénticas (unos 10 segundos). Termina
con código 0 si todas coinciden.

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

- Los algoritmos y los generadores solo aceptan coordenadas `int` o `double`:
  un `static_assert` en `EstrategiaCerradura` y en `GeneradorPuntos` rechaza
  cualquier otro tipo al compilar. Con otros tipos el código compilaría pero
  daría resultados incorrectos sin aviso: `long double` se redondea a `double`
  dentro de `orientacion`, `long long` no tiene un tipo más ancho donde
  calcular el producto cruz y se desborda, y `short` o `unsigned` no pueden
  representar las coordenadas de los generadores (con signo, hasta 1.42·10^8).
  `Punto`, `Vector` y `Poligono` siguen siendo genéricos, porque los
  predicados usan `Punto<long long>` internamente.
- Con coordenadas enteras, |coordenadas| < 2^28: así el producto cruz, que se
  calcula en `long long`, es exacto también en los cálculos con el centroide
  del Random Incremental. Los generadores usan |coordenadas| ≤ 1.42·10^8.
- Con `double` el producto cruz se redondea; con puntos casi colineales los
  algoritmos pueden no coincidir (ver `ANALISIS.pdf`, sección de
  colinealidad).

## Agregados a la Tarea 1

- `Punto::operator<`: orden lexicográfico exacto, para ordenar.
- `Poligono()`: polígono vacío; `Poligono::normalizar()`: forma canónica
  (CCW, desde el menor vértice); `Poligono::operator==` y `operator!=`.

## Declaración de uso de IA

Se utilizó Claude (Opus 5.5) para complementar y revisar el código, los tests y el
experimento, para discutir conceptos e ideas de diseño, y para redactar el
informe y la documentación.

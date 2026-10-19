/**
 * @file exactitud.cpp
 * @brief Vuelca las nubes del caso "cuadrado rotado 30 grados, double" del
 *        experimento y las cerraduras de ambos algoritmos, para exactitud.py.
 *
 * Uso, desde la raiz:
 *   g++ -std=c++17 -O2 -Iinclude analisis/exactitud.cpp -o build/exactitud
 *   ./build/exactitud build/exactitud_datos
 *
 * Las coordenadas se escriben en hexadecimal (%a), que es exacto: exactitud.py
 * recalcula la cerradura con aritmetica racional sobre los mismos double.
 */

#include <cstdint>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

#include "GeneradorPuntos.hpp"
#include "IncrementalAlgorithm.hpp"
#include "RandomIncrementalAlgorithm.hpp"

/**
 * @brief Escribe puntos con una letra al inicio de cada linea.
 * @param f Archivo de salida.
 * @param letra P (nube), I (Incremental) o R (Random Incremental).
 * @param puntos Los puntos.
 */
static void escribir(std::FILE *f, char letra, const std::vector<Punto<double>> &puntos) {
  for (const Punto<double> &q : puntos) {
    std::fprintf(f, "%c %a %a\n", letra, q.getX(), q.getY());
  }
}

/**
 * @brief Vertices de un poligono como vector.
 * @param c El poligono.
 * @return Sus vertices, en orden.
 */
static std::vector<Punto<double>> vertices(const Poligono<double> &c) {
  std::vector<Punto<double>> v;
  for (int i = 0; i < c.cantidadPuntos(); ++i) {
    v.push_back(c[i]);
  }
  return v;
}

/**
 * @brief Genera las mismas nubes que la parte "colinealidad" del experimento.
 * @param argc Cantidad de argumentos.
 * @param argv argv[1]: directorio de salida (debe existir).
 * @return 0 al terminar.
 */
int main(int argc, char **argv) {
  const std::string dir = argc > 1 ? argv[1] : ".";
  GeneradorCuadrado<double> rotado(0.1, 3.14159265358979323846 / 6);
  for (std::size_t n : {10000, 100000}) {
    for (int i = 0; i < 10; ++i) {
      std::mt19937_64 rng(1000003ULL * i + n); // las semillas de experimento.cpp
      const std::vector<Punto<double>> nube = rotado.generar(n, rng);
      IncrementalAlgorithm<double> incremental;
      RandomIncrementalAlgorithm<double> aleatorio(i + 1);
      const std::string ruta = dir + "/" + std::to_string(n) + "_" + std::to_string(i) + ".txt";
      std::FILE *f = std::fopen(ruta.c_str(), "w");
      escribir(f, 'P', nube);
      escribir(f, 'I', vertices(incremental.apply(nube)));
      escribir(f, 'R', vertices(aleatorio.apply(nube)));
      std::fclose(f);
    }
  }
  return 0;
}

/**
 * @file main.cpp
 * @brief Ejemplo de uso de las estrategias y comparacion rapida entre ellas.
 *
 * Primero calcula la cerradura de una nube pequena con ambas estrategias.
 * Luego, para cada generador y cada n, mide ambas sobre la misma nube y
 * verifica que las dos cerraduras sean identicas. El experimento completo,
 * con varias instancias por caso, esta en experimento.cpp.
 */

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "GeneradorPuntos.hpp"
#include "IncrementalAlgorithm.hpp"
#include "Medicion.hpp"
#include "RandomIncrementalAlgorithm.hpp"

/**
 * @brief Mide ambas estrategias sobre una nube generada e imprime una fila.
 * @tparam T Tipo de las coordenadas.
 * @param tipo Nombre del tipo, para la tabla.
 * @param caso Descripcion del generador, para la tabla.
 * @param g Generador de la nube.
 * @param n Cantidad de puntos.
 * @return true si las dos cerraduras son identicas.
 */
template <typename T>
bool compararUnaVez(const std::string &tipo, const std::string &caso,
                    const GeneradorPuntos<T> &g, std::size_t n) {
  std::mt19937_64 rng(n);
  const std::vector<Punto<T>> nube = g.generar(n, rng);

  IncrementalAlgorithm<T> incremental;
  RandomIncrementalAlgorithm<T> aleatorio(n);
  Poligono<T> a, b;
  const double tA = medirMs([&] { a = incremental.apply(nube); });
  const double tB = medirMs([&] { b = aleatorio.apply(nube); });

  const bool iguales = (a == b);
  std::cout << std::setw(6) << tipo << std::setw(22) << caso << std::setw(9) << n
            << std::setw(9) << a.cantidadPuntos() << std::setw(14) << tA
            << std::setw(14) << tB << std::setw(10) << (iguales ? "si" : "NO")
            << "\n";
  return iguales;
}

/**
 * @brief Recorre los generadores y tamanos de la comparacion rapida.
 * @tparam T Tipo de las coordenadas.
 * @param tipo Nombre del tipo, para la tabla.
 * @return true si en todos los casos las cerraduras son identicas.
 */
template <typename T> bool compararTodo(const std::string &tipo) {
  GeneradorUniforme<T> uniforme;
  GeneradorCircunferencia<T> uno(0.01), diez(0.10), todos(1.0);
  GeneradorCuadrado<T> cuadrado(0.10, 0.0);
  const std::vector<std::pair<std::string, const GeneradorPuntos<T> *>> casos = {
      {"uniforme", &uniforme},
      {"circunferencia 1%", &uno},
      {"circunferencia 10%", &diez},
      {"circunferencia 100%", &todos},
      {"cuadrado 10% (colin.)", &cuadrado}};

  bool todasIguales = true;
  for (const auto &[caso, g] : casos) {
    for (std::size_t n : {10000, 100000, 1000000}) {
      todasIguales &= compararUnaVez(tipo, caso, *g, n);
    }
  }
  return todasIguales;
}

/**
 * @brief Ejemplo pequeno y comparacion rapida de las dos estrategias.
 * @return 0 si todas las cerraduras coinciden, 1 si alguna difiere.
 */
int main() {
  // Ejemplo: el cuadrado (0,0)-(4,4) con puntos dentro y sobre sus lados.
  const std::vector<Punto<int>> nube = {{2, 2}, {4, 4}, {0, 0}, {4, 0}, {1, 3},
                                        {0, 4}, {2, 0}, {4, 2}, {0, 0}};
  std::vector<std::unique_ptr<EstrategiaCerradura<int>>> estrategias;
  estrategias.push_back(std::make_unique<IncrementalAlgorithm<int>>());
  estrategias.push_back(std::make_unique<RandomIncrementalAlgorithm<int>>(1));
  std::cout << "nube: " << Poligono<int>(nube) << "\n";
  for (const auto &e : estrategias) {
    std::cout << e->nombre() << ": " << e->apply(nube) << "\n";
  }

  std::cout << "\nComparacion (una instancia por caso, tiempos en ms)\n"
            << std::setw(6) << "tipo" << std::setw(22) << "generador"
            << std::setw(9) << "n" << std::setw(9) << "h" << std::setw(14)
            << "incremental" << std::setw(14) << "random_incr." << std::setw(10)
            << "iguales" << "\n"
            << std::fixed << std::setprecision(3);
  const bool igualesDouble = compararTodo<double>("double");
  const bool igualesInt = compararTodo<int>("int");
  const bool iguales = igualesDouble && igualesInt;
  std::cout << (iguales ? "\nTodas las cerraduras son identicas.\n"
                        : "\nHay cerraduras distintas.\n");
  return iguales ? 0 : 1;
}

/**
 * @file Medicion.hpp
 * @brief Medicion de tiempos de ejecucion.
 */

#pragma once

#include <chrono>

/**
 * @brief Tiempo promedio de una ejecucion de f, en milisegundos.
 * @tparam F Funcion sin argumentos.
 * @param f Lo que se mide.
 * @param minimoMs Tiempo total minimo: f se repite hasta juntarlo, para que
 *        con n pequeno la resolucion del reloj no domine la medicion.
 * @return El tiempo total dividido por la cantidad de repeticiones.
 */
template <typename F> double medirMs(F &&f, double minimoMs = 50.0) {
  using Reloj = std::chrono::steady_clock;
  const auto inicio = Reloj::now();
  int repeticiones = 0;
  double total = 0;
  do {
    f();
    ++repeticiones;
    total = std::chrono::duration<double, std::milli>(Reloj::now() - inicio).count();
  } while (total < minimoMs);
  return total / repeticiones;
}

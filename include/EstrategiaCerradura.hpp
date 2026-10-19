/**
 * @file EstrategiaCerradura.hpp
 * @brief Interfaz comun de los algoritmos de cerradura convexa (patron Strategy).
 */

#pragma once

#include <string>
#include <vector>

#include "Poligono.hpp"
#include "Punto.hpp"

/**
 * @brief Estrategia de calculo de la cerradura convexa de una nube de puntos.
 * @tparam T Tipo numerico de las coordenadas.
 *
 * Cada algoritmo hereda de esta clase y sobreescribe apply. El codigo que
 * mide y compara (main, experimento) solo conoce esta interfaz.
 *
 * Convencion de la salida, igual en todas las estrategias:
 * - solo los vertices extremos: un punto colineal sobre una arista no es
 *   vertice, y los repetidos aparecen una vez;
 * - en sentido antihorario (CCW), comenzando en el menor vertice segun el
 *   orden lexicografico (ver Poligono::normalizar);
 * - con menos de 3 puntos no colineales la cerradura es degenerada: vacia,
 *   un punto, o el segmento entre los dos extremos.
 */
template <typename T> class EstrategiaCerradura {
public:
  /** @brief Destructor virtual: las estrategias se usan por puntero a la base. */
  virtual ~EstrategiaCerradura() = default;

  /**
   * @brief Calcula la cerradura convexa.
   * @param nube Puntos de entrada, en cualquier orden y con posibles repetidos.
   * @return La cerradura, con la convencion descrita en la clase.
   */
  virtual Poligono<T> apply(const std::vector<Punto<T>> &nube) = 0;

  /** @brief Nombre corto de la estrategia, para tablas y archivos de datos. */
  virtual std::string nombre() const = 0;
};

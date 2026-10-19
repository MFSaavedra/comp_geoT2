/**
 * @file IncrementalAlgorithm.hpp
 * @brief Algoritmo incremental: ordenar, construir el upper hull y luego el
 *        lower hull (de Berg et al., seccion 1.1; "monotone chain" de Andrew).
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "EstrategiaCerradura.hpp"
#include "Geometria.hpp"

/**
 * @brief Cerradura convexa incremental sobre los puntos ordenados, O(n log n).
 * @tparam T Tipo numerico de las coordenadas.
 *
 * Al recorrer los puntos de izquierda a derecha, cada punto nuevo es el
 * extremo derecho de los ya vistos, asi que siempre es vertice de su
 * cerradura: basta agregarlo al final y sacar los puntos que dejan de formar
 * una vuelta a la derecha. Ordenar cuesta O(n log n) y cada recorrido O(n),
 * porque cada punto entra y sale de la cadena a lo mas una vez.
 */
template <typename T> class IncrementalAlgorithm : public EstrategiaCerradura<T> {
public:
  /**
   * @brief Calcula la cerradura convexa de la nube.
   * @param nube Puntos de entrada.
   * @return La cerradura, con la convencion de EstrategiaCerradura.
   */
  Poligono<T> apply(const std::vector<Punto<T>> &nube) override {
    std::vector<Punto<T>> p(nube);
    std::sort(p.begin(), p.end()); // por x, y a igual x por y
    p.erase(std::unique(p.begin(), p.end(), mismoPunto<T>), p.end());
    if (p.size() < 3) {
      return Poligono<T>(p); // vacio, un punto o un segmento, ya ordenados
    }

    // Upper hull, de izquierda a derecha. Por arriba, avanzar hacia la
    // derecha es avanzar en CW: solo se aceptan vueltas a la derecha, y un
    // punto colineal (orientacion 0) tambien se saca.
    std::vector<Punto<T>> cadena;
    for (const Punto<T> &q : p) {
      while (cadena.size() >= 2 &&
             orientacion(cadena[cadena.size() - 2], cadena.back(), q) >= 0) {
        cadena.pop_back();
      }
      cadena.push_back(q);
    }

    // Lower hull, devolviendose de derecha a izquierda desde el extremo
    // derecho, que ya esta en la cadena y no se puede sacar.
    const std::size_t base = cadena.size();
    for (auto q = p.rbegin() + 1; q != p.rend(); ++q) {
      while (cadena.size() > base &&
             orientacion(cadena[cadena.size() - 2], cadena.back(), *q) >= 0) {
        cadena.pop_back();
      }
      cadena.push_back(*q);
    }
    cadena.pop_back(); // el extremo izquierdo quedo repetido al final

    Poligono<T> cerradura(cadena); // en CW
    cerradura.normalizar();
    return cerradura;
  }

  /** @brief Nombre de la estrategia. */
  std::string nombre() const override { return "incremental"; }
};

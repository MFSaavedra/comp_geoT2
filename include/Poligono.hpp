/**
 * @file Poligono.hpp
 * @brief Clase Poligono: poligono en un espacio bidimensional.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ostream>
#include <type_traits>
#include <vector>

#include "Punto.hpp"

/**
 * @brief Poligono en el plano, dado por sus vertices en orden.
 * @tparam T Tipo numerico de las coordenadas.
 */
template <typename T> class Poligono {
private:
  std::vector<Punto<T>> puntos; ///< Vertices en el orden dado.

  /**
   * @brief Area con signo segun la formula del shoelace.
   * @return Positiva si los vertices van en CCW, negativa si van en CW.
   * @note Base de esCCW, hacerCCW y area. Promueve a double antes de
   *       multiplicar: con T entero, coordenadas grandes desbordarian. Con
   *       long double suma en long double: en double, vertices que solo
   *       difieren despues del bit 53 se confundirian, y hacerCCW podria no
   *       dar vuelta un poligono en CW.
   */
  double areaConSigno() const {
    using R = std::common_type_t<T, double>; // double, o long double
    R suma = 0;
    const std::size_t n = puntos.size();
    for (std::size_t i = 0; i < n; ++i) {
      const Punto<T> &a = puntos[i];
      const Punto<T> &b = puntos[(i + 1) % n]; // el ultimo cierra con el primero
      suma += static_cast<R>(a.getX()) * static_cast<R>(b.getY()) -
              static_cast<R>(b.getX()) * static_cast<R>(a.getY());
    }
    return static_cast<double>(suma / 2);
  }

public:
  /** @brief Construye un poligono sin vertices. */
  Poligono() = default;

  /**
   * @brief Construye el poligono con los puntos dados, en ese orden.
   * @param puntos Vertices del poligono.
   */
  explicit Poligono(const std::vector<Punto<T>> &puntos) : puntos(puntos) {}

  /**
   * @brief Retorna la cantidad de puntos del poligono.
   * @note int y no size_t para calzar con el indice de operator[].
   */
  int cantidadPuntos() const { return static_cast<int>(puntos.size()); }

  /**
   * @brief Indica si los vertices estan en sentido antihorario (CCW).
   * @return true si el area con signo es positiva.
   */
  bool esCCW() const { return areaConSigno() > 0; }

  /** @brief Si el poligono esta en CW, invierte sus vertices para dejarlo en CCW. */
  void hacerCCW() {
    if (areaConSigno() < 0) {
      std::reverse(puntos.begin(), puntos.end());
    }
  }

  /**
   * @brief Area del poligono.
   * @return El area en double, sin signo.
   */
  double area() const { return std::fabs(areaConSigno()); }

  /**
   * @brief Deja el poligono en CCW y comenzando en su menor vertice
   *        (orden lexicografico de Punto).
   * @note Forma canonica: dos cerraduras convexas del mismo conjunto quedan
   *       con los mismos vertices en el mismo orden, y se comparan con ==.
   */
  void normalizar() {
    hacerCCW();
    std::rotate(puntos.begin(), std::min_element(puntos.begin(), puntos.end()),
                puntos.end());
  }

  /**
   * @brief Compara dos poligonos vertice a vertice, en el orden guardado.
   * @param other Poligono a comparar.
   * @return true si tienen los mismos vertices en el mismo orden.
   */
  bool operator==(const Poligono<T> &other) const {
    return puntos == other.puntos;
  }

  /**
   * @brief Verifica si 2 poligonos son distintos.
   * @param other Poligono a comparar.
   * @return true si difieren en algun vertice o en la cantidad.
   */
  bool operator!=(const Poligono<T> &other) const { return !(*this == other); }

  /**
   * @brief Acceso al punto en la posicion i.
   * @param i Indice del punto, desde 0.
   * @return Referencia al punto, modificable.
   */
  Punto<T> &operator[](int i) { return puntos[i]; }

  /**
   * @brief Acceso de solo lectura al punto en la posicion i.
   * @param i Indice del punto, desde 0.
   * @return Referencia constante al punto.
   */
  const Punto<T> &operator[](int i) const { return puntos[i]; }

  /**
   * @brief Imprime el poligono en formato [(x1,y1),(x2,y2),...,(xn,yn)].
   * @param os Flujo de salida.
   * @param p Poligono a imprimir.
   * @return El mismo flujo os.
   */
  friend std::ostream &operator<<(std::ostream &os, const Poligono<T> &p) {
    const char *sep = "";
    os << "[";
    for (const Punto<T> &q : p.puntos) {
      os << sep << q;
      sep = ",";
    }
    return os << "]";
  }
};

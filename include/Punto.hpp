/**
 * @file Punto.hpp
 * @brief Clase Punto: punto en un espacio bidimensional.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <ostream>
#include <type_traits>

/**
 * @brief Punto en el plano, con una coordenada z auxiliar.
 * @tparam T Tipo numerico de las coordenadas.
 */
template <typename T> class Punto {
private:
  T x; ///< Coordenada x.
  T y; ///< Coordenada y.
  T z; ///< Coordenada z auxiliar; recibe el resultado de productoCruz.

  /**
   * @brief Igualdad tolerante en punto flotante, exacta en enteros.
   * @param a Primer operando.
   * @param b Segundo operando.
   * @return true si son iguales dentro de la tolerancia.
   */
  static bool casiIgual(T a, T b) {
    if constexpr (std::is_floating_point_v<T>) {
      constexpr T eps = std::numeric_limits<T>::epsilon() * 100;
      const T diff = std::fabs(a - b);
      // Absoluto cerca de cero, relativo para el resto.
      if (diff <= eps) {
        return true;
      }
      return diff <= eps * std::max(std::fabs(a), std::fabs(b));
    } else {
      return a == b;
    }
  }

public:
  /** @brief Construye el punto en el origen. */
  Punto() : x(T{}), y(T{}), z(T{}) {}

  /**
   * @brief Construye un punto 2D, con z = 0.
   * @param x Coordenada x.
   * @param y Coordenada y.
   */
  Punto(T x, T y) : x(x), y(y), z(T{}) {}

  /**
   * @brief Construye un punto 3D.
   * @param x Coordenada x.
   * @param y Coordenada y.
   * @param z Coordenada z.
   */
  Punto(T x, T y, T z) : x(x), y(y), z(z) {}

  /**
   * @brief Constructor de copia.
   * @param other Punto a copiar.
   */
  Punto(const Punto<T> &other) : x(other.x), y(other.y), z(other.z) {}

  /**
   * @brief Asignacion por copia.
   * @param other Punto a copiar.
   * @return Referencia a este punto.
   */
  Punto<T> &operator=(const Punto<T> &other) = default;

  // getters
  /** @brief Retorna la coordenada x. */
  T getX() const { return x; }

  /** @brief Retorna la coordenada y. */
  T getY() const { return y; }

  /** @brief Retorna la coordenada z. */
  T getZ() const { return z; }

  /**
   * @brief Distancia euclidiana entre este punto y otro.
   * @param other Punto destino.
   * @return La distancia en double.
   * @note Retorna double y promueve antes de restar: con T entero, truncar
   *       daria 1 en vez de 1.41..., y las coordenadas grandes desbordarian.
   *       std::hypot evita el overflow del cuadrado intermedio.
   */
  double distancia(const Punto<T> &other) const {
    const double dx = static_cast<double>(x) - static_cast<double>(other.x);
    const double dy = static_cast<double>(y) - static_cast<double>(other.y);
    const double dz = static_cast<double>(z) - static_cast<double>(other.z);
    return std::hypot(dx, dy, dz);
  }

  /**
   * @brief Compara dos puntos componente a componente.
   * @param other Punto a comparar.
   * @return true si todas las coordenadas coinciden.
   */
  bool operator==(const Punto<T> &other) const {
    return casiIgual(x, other.x) && casiIgual(y, other.y) &&
           casiIgual(z, other.z);
  }

  /**
   * @brief Verifica si 2 puntos son distintos.
   * @param other Punto a comparar.
   * @return true si difieren en alguna coordenada.
   */
  bool operator!=(const Punto<T> &other) const { return !(*this == other); }

  /**
   * @brief Orden lexicografico: primero x, luego y.
   * @param other Punto a comparar.
   * @return true si este punto va antes que other.
   * @note Exacto, sin la tolerancia de operator==: std::sort necesita un orden
   *       estricto consistente.
   */
  bool operator<(const Punto<T> &other) const {
    return x < other.x || (x == other.x && y < other.y);
  }

  /**
   * @brief Imprime el punto en formato (x,y).
   * @param os Flujo de salida.
   * @param p Punto a imprimir.
   * @return El mismo flujo os.
   */
  friend std::ostream &operator<<(std::ostream &os, const Punto<T> &p) {
    return os << "(" << p.x << "," << p.y << ")";
  }
};

/**
 * @file GeneradorPuntos.hpp
 * @brief Generadores de nubes de puntos de prueba (patron Strategy).
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include "Punto.hpp"

/**
 * @brief Estrategia de generacion de una nube de puntos de prueba.
 * @tparam T Tipo numerico de las coordenadas.
 *
 * Todos los generadores trabajan en double dentro del disco de radio
 * RADIO = 10^8 y luego convierten a T: con T entero se redondea al entero mas
 * cercano. Asi un mismo generador con la misma semilla produce la misma
 * figura en int y en double, y la unica diferencia es el redondeo.
 */
template <typename T> class GeneradorPuntos {
public:
  /** @brief Destructor virtual: los generadores se usan por puntero a la base. */
  virtual ~GeneradorPuntos() = default;

  /**
   * @brief Genera la nube.
   * @param n Cantidad de puntos.
   * @param rng Generador aleatorio; con la misma semilla se obtiene la misma nube.
   * @return Los n puntos, en orden aleatorio.
   */
  virtual std::vector<Punto<T>> generar(std::size_t n,
                                        std::mt19937_64 &rng) const = 0;

  /** @brief Nombre corto del generador, para tablas y archivos de datos. */
  virtual std::string nombre() const = 0;

protected:
  /// Escala de las coordenadas. Con T entero, |coordenadas| < 2^28 (ver
  /// RandomIncrementalAlgorithm), incluso en las esquinas del cuadrado.
  static constexpr double RADIO = 1e8;
  static constexpr double PI = 3.14159265358979323846; ///< El numero pi.

  /**
   * @brief Convierte coordenadas double al tipo T.
   * @param x Coordenada x.
   * @param y Coordenada y.
   * @return El punto, redondeado si T es entero.
   */
  static Punto<T> crear(double x, double y) {
    if constexpr (std::is_integral_v<T>) {
      return Punto<T>(static_cast<T>(std::llround(x)),
                      static_cast<T>(std::llround(y)));
    } else {
      return Punto<T>(static_cast<T>(x), static_cast<T>(y));
    }
  }

  /**
   * @brief Cantidad de puntos que corresponde a un porcentaje.
   * @param porcentaje Fraccion entre 0 y 1.
   * @param n Total de puntos.
   * @param minimo Cantidad minima (la figura necesita al menos esos puntos).
   * @return round(porcentaje * n), acotado a [minimo, n].
   */
  static std::size_t cantidad(double porcentaje, std::size_t n,
                              std::size_t minimo) {
    const auto k = static_cast<std::size_t>(std::llround(porcentaje * n));
    return std::min(n, std::max(minimo, k));
  }
};

/**
 * @brief Puntos uniformes en el cuadrado [-RADIO, RADIO]^2.
 * @tparam T Tipo numerico de las coordenadas.
 * @note La cerradura esperada tiene unos (8/3) ln n vertices: es el caso con
 *       un porcentaje muy pequeno de puntos en la cerradura.
 */
template <typename T> class GeneradorUniforme : public GeneradorPuntos<T> {
public:
  /**
   * @brief Genera n puntos uniformes en el cuadrado.
   * @param n Cantidad de puntos.
   * @param rng Generador aleatorio.
   * @return Los puntos.
   */
  std::vector<Punto<T>> generar(std::size_t n,
                                std::mt19937_64 &rng) const override {
    std::uniform_real_distribution<double> u(-this->RADIO, this->RADIO);
    std::vector<Punto<T>> puntos;
    puntos.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      const double x = u(rng); // en dos lineas: el orden de evaluacion
      const double y = u(rng); // de los argumentos no esta definido
      puntos.push_back(this->crear(x, y));
    }
    return puntos;
  }

  /** @brief Nombre del generador. */
  std::string nombre() const override { return "uniforme"; }
};

/**
 * @brief Un porcentaje de los puntos sobre una circunferencia y el resto
 *        uniforme en el interior: la cerradura no tiene puntos colineales.
 * @tparam T Tipo numerico de las coordenadas.
 *
 * Los k puntos de la circunferencia de radio RADIO estan equiespaciados (con
 * un desfase aleatorio), asi que forman un k-gono regular. El resto se
 * distribuye uniforme en el disco inscrito en ese k-gono, de radio
 * RADIO cos(pi / k), con un margen de 0.1%: quedan estrictamente dentro y la
 * cerradura tiene exactamente k vertices. Con T entero el redondeo puede
 * romper la posicion convexa de los k puntos (ver ANALISIS.pdf).
 */
template <typename T> class GeneradorCircunferencia : public GeneradorPuntos<T> {
public:
  /**
   * @brief Construye el generador.
   * @param porcentaje Fraccion de los puntos que forma la cerradura, en [0, 1].
   */
  explicit GeneradorCircunferencia(double porcentaje) : porcentaje(porcentaje) {}

  /**
   * @brief Genera n puntos, de los cuales max(3, porcentaje * n) estan en la
   *        circunferencia.
   * @param n Cantidad de puntos.
   * @param rng Generador aleatorio.
   * @return Los puntos, barajados.
   */
  std::vector<Punto<T>> generar(std::size_t n,
                                std::mt19937_64 &rng) const override {
    const double R = this->RADIO, pi = this->PI;
    const std::size_t k = this->cantidad(porcentaje, n, 3);
    std::uniform_real_distribution<double> u01(0.0, 1.0);
    std::vector<Punto<T>> puntos;
    puntos.reserve(n);

    const double desfase = 2 * pi * u01(rng);
    for (std::size_t i = 0; i < k; ++i) {
      const double angulo = desfase + 2 * pi * i / k;
      puntos.push_back(this->crear(R * std::cos(angulo), R * std::sin(angulo)));
    }

    const double radioInterior = 0.999 * R * std::cos(pi / k);
    for (std::size_t i = k; i < n; ++i) {
      const double rho = radioInterior * std::sqrt(u01(rng)); // uniforme en area
      const double angulo = 2 * pi * u01(rng);
      puntos.push_back(this->crear(rho * std::cos(angulo), rho * std::sin(angulo)));
    }

    std::shuffle(puntos.begin(), puntos.end(), rng);
    return puntos;
  }

  /** @brief Nombre del generador. */
  std::string nombre() const override { return "circunferencia"; }

private:
  double porcentaje; ///< Fraccion de los puntos sobre la circunferencia.
};

/**
 * @brief Un porcentaje de los puntos sobre el borde de un cuadrado y el resto
 *        uniforme en su interior: la cerradura tiene muchos puntos colineales.
 * @tparam T Tipo numerico de las coordenadas.
 *
 * El cuadrado tiene lado 2 RADIO y esta rotado en el angulo dado. Las 4
 * esquinas siempre estan; los demas puntos del borde caen en sus lados. Con
 * angulo 0 los lados son horizontales y verticales y la colinealidad es
 * exacta en int y en double. Con otro angulo la colinealidad es exacta solo
 * en los reales: en double los puntos quedan a un redondeo de la recta, y en
 * int a medio entero.
 */
template <typename T> class GeneradorCuadrado : public GeneradorPuntos<T> {
public:
  /**
   * @brief Construye el generador.
   * @param porcentaje Fraccion de los puntos sobre el borde, en [0, 1].
   * @param angulo Rotacion del cuadrado, en radianes.
   */
  GeneradorCuadrado(double porcentaje, double angulo)
      : porcentaje(porcentaje), angulo(angulo) {}

  /**
   * @brief Genera n puntos, de los cuales max(4, porcentaje * n) estan en el borde.
   * @param n Cantidad de puntos.
   * @param rng Generador aleatorio.
   * @return Los puntos, barajados.
   */
  std::vector<Punto<T>> generar(std::size_t n,
                                std::mt19937_64 &rng) const override {
    const double R = this->RADIO;
    const std::size_t k = this->cantidad(porcentaje, n, 4);
    std::uniform_real_distribution<double> lado(-R, R);
    std::uniform_real_distribution<double> dentro(-0.999 * R, 0.999 * R);
    std::uniform_int_distribution<int> cualLado(0, 3);
    std::vector<Punto<T>> puntos;
    puntos.reserve(n);

    const double esquinas[4][2] = {{-R, -R}, {R, -R}, {R, R}, {-R, R}};
    for (std::size_t i = 0; i < n; ++i) {
      double x, y;
      if (i < 4 && i < k) {
        x = esquinas[i][0];
        y = esquinas[i][1];
      } else if (i < k) {
        const double t = lado(rng);
        switch (cualLado(rng)) {
        case 0: x = t, y = -R; break;
        case 1: x = R, y = t; break;
        case 2: x = t, y = R; break;
        default: x = -R, y = t; break;
        }
      } else {
        x = dentro(rng);
        y = dentro(rng);
      }
      puntos.push_back(rotado(x, y));
    }

    std::shuffle(puntos.begin(), puntos.end(), rng);
    return puntos;
  }

  /** @brief Nombre del generador. */
  std::string nombre() const override { return "cuadrado"; }

private:
  double porcentaje; ///< Fraccion de los puntos sobre el borde.
  double angulo;     ///< Rotacion del cuadrado, en radianes.

  /**
   * @brief Rota (x, y) en el angulo del generador y lo convierte a T.
   * @param x Coordenada x sin rotar.
   * @param y Coordenada y sin rotar.
   * @return El punto rotado.
   * @note Con angulo 0, cos = 1 y sin = 0 dan exactamente (x, y).
   */
  Punto<T> rotado(double x, double y) const {
    const double c = std::cos(angulo), s = std::sin(angulo);
    return this->crear(c * x - s * y, s * x + c * y);
  }
};

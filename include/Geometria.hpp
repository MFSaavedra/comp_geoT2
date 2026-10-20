/**
 * @file Geometria.hpp
 * @brief Predicados geometricos que comparten los algoritmos de cerradura.
 */

#pragma once

#include <type_traits>

#include "Punto.hpp"
#include "Vector.hpp"

/**
 * @brief Indica si T es uno de los tipos de coordenadas que aceptan los
 *        algoritmos y los generadores.
 * @tparam T Tipo a revisar.
 * @note Son los tipos para los que estan pensados Ancho y los generadores (ver
 *       README, seccion Supuestos). Otros, como char o long, compilarian pero
 *       no estan revisados.
 */
template <typename T>
inline constexpr bool esTipoCoordenada =
    std::is_same_v<T, short> || std::is_same_v<T, int> ||
    std::is_same_v<T, unsigned> || std::is_same_v<T, long long> ||
    std::is_same_v<T, float> || std::is_same_v<T, double> ||
    std::is_same_v<T, long double>;

/// Entero de 128 bits (extension de GCC y Clang): el producto cruz de
/// coordenadas de 64 bits no cabe en long long. __extension__ evita el aviso
/// de -Wpedantic.
__extension__ typedef __int128 Entero128;

/**
 * @brief Tipo en que se calcula el producto cruz de coordenadas de tipo T.
 * @tparam T Tipo de las coordenadas.
 * @note Con T entero es al menos el doble de ancho que T: long long si T
 *       tiene hasta 32 bits, Entero128 si tiene 64. El resultado es exacto
 *       mientras, en cada eje, max - min < 2^31 o < 2^63 respectivamente.
 *       Con T flotante es el mas preciso entre T y double, asi que nunca pierde
 *       precision respecto de T (long double sigue en long double); hay
 *       redondeo.
 */
template <typename T>
using Ancho = std::conditional_t<
    std::is_floating_point_v<T>, std::common_type_t<T, double>,
    std::conditional_t<(sizeof(T) <= 4), long long, Entero128>>;

/**
 * @brief Signo del producto cruz u x v.
 * @tparam W Tipo de las componentes, ya promovido a uno donde el producto no
 *         se desborda.
 * @param u Primer vector.
 * @param v Segundo vector.
 * @return 1 si v esta a la izquierda de u (giro CCW), -1 si esta a la derecha
 *         (giro CW) y 0 si son paralelos.
 */
template <typename W> int signoCruz(const Vector<W> &u, const Vector<W> &v) {
  const W cruz = u.productoCruz(v).getZ();
  return (cruz > 0) - (cruz < 0);
}

/**
 * @brief Orientacion del triangulo (a, b, c): signo del producto cruz
 *        (b - a) x (c - a).
 * @tparam T Tipo de las coordenadas.
 * @param a Primer punto.
 * @param b Segundo punto.
 * @param c Punto a clasificar respecto de la recta a -> b.
 * @return 1 si c esta a la izquierda de a -> b (giro CCW), -1 si esta a la
 *         derecha (giro CW) y 0 si los tres puntos son colineales.
 * @note Promueve a Ancho<T> antes de restar: con T = int, Vector<int> se
 *       desborda apenas las coordenadas pasan de 2^15, y con T = unsigned
 *       restar en T daria la vuelta (3 - 5 = 2^32 - 2).
 */
template <typename T>
int orientacion(const Punto<T> &a, const Punto<T> &b, const Punto<T> &c) {
  using W = Ancho<T>;
  const Vector<W> ab(W(b.getX()) - W(a.getX()), W(b.getY()) - W(a.getY()));
  const Vector<W> ac(W(c.getX()) - W(a.getX()), W(c.getY()) - W(a.getY()));
  return signoCruz(ab, ac);
}

/**
 * @brief Igualdad exacta de las coordenadas x e y.
 * @tparam T Tipo de las coordenadas.
 * @param a Primer punto.
 * @param b Segundo punto.
 * @return true si a y b tienen exactamente las mismas coordenadas.
 * @note A diferencia de Punto::operator==, no usa tolerancia: los algoritmos
 *       solo descartan repetidos exactos, igual que orientacion los ve.
 */
template <typename T> bool mismoPunto(const Punto<T> &a, const Punto<T> &b) {
  return a.getX() == b.getX() && a.getY() == b.getY();
}

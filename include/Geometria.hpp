/**
 * @file Geometria.hpp
 * @brief Predicados geometricos que comparten los algoritmos de cerradura.
 */

#pragma once

#include <type_traits>

#include "Punto.hpp"
#include "Vector.hpp"

/**
 * @brief Tipo en que se calcula el producto cruz de coordenadas de tipo T.
 * @tparam T Tipo de las coordenadas.
 * @note Con T entero es long long, y el resultado es exacto mientras
 *       |coordenadas| < 2^30. Con T flotante es double y hay redondeo.
 */
template <typename T>
using Ancho = std::conditional_t<std::is_integral_v<T>, long long, double>;

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
 *       desborda apenas las coordenadas pasan de 2^15.
 */
template <typename T>
int orientacion(const Punto<T> &a, const Punto<T> &b, const Punto<T> &c) {
  using W = Ancho<T>;
  const Vector<W> ab(W(b.getX()) - W(a.getX()), W(b.getY()) - W(a.getY()));
  const Vector<W> ac(W(c.getX()) - W(a.getX()), W(c.getY()) - W(a.getY()));
  const W cruz = ab.productoCruz(ac).getZ();
  return (cruz > 0) - (cruz < 0);
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

/**
 * @file Geometria_test.cpp
 * @brief Tests de los predicados de Geometria.hpp.
 */

#include <gtest/gtest.h>

#include "Geometria.hpp"

TEST(Geometria, Orientacion) {
  const Punto<int> a(0, 0), b(4, 0);
  EXPECT_EQ(orientacion(a, b, Punto<int>(2, 3)), 1);  // izquierda: CCW
  EXPECT_EQ(orientacion(a, b, Punto<int>(2, -3)), -1); // derecha: CW
  EXPECT_EQ(orientacion(a, b, Punto<int>(9, 0)), 0);   // colineal
  EXPECT_EQ(orientacion(Punto<double>(0, 0), Punto<double>(1, 1),
                        Punto<double>(0.5, 0.6)),
            1);
}

TEST(Geometria, OrientacionSinDesborde) {
  // Con int, el producto cruz de estas diferencias (~2^60) desbordaria.
  const int g = 1 << 29;
  const Punto<int> a(-g, -g), b(g, -g);
  EXPECT_EQ(orientacion(a, b, Punto<int>(g, g)), 1);
  EXPECT_EQ(orientacion(a, b, Punto<int>(0, -g)), 0);
  EXPECT_EQ(orientacion(a, b, Punto<int>(0, -g - 1)), -1);
}

TEST(Geometria, MismoPunto) {
  EXPECT_TRUE(mismoPunto(Punto<int>(1, 2), Punto<int>(1, 2)));
  EXPECT_FALSE(mismoPunto(Punto<int>(1, 2), Punto<int>(2, 1)));
  EXPECT_FALSE(mismoPunto(Punto<double>(0.1 + 0.2, 0), Punto<double>(0.3, 0)));
  EXPECT_TRUE(mismoPunto(Punto<int>(1, 2), Punto<int>(1, 2, 7))); // ignora z
}

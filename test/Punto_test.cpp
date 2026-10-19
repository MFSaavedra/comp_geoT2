/**
 * @file Punto_test.cpp
 * @brief Tests de la clase Punto.
 */

#include <cmath>
#include <sstream>

#include <gtest/gtest.h>

#include "Punto.hpp"

TEST(Punto, Constructores) {
  Punto<int> p(1, 2);
  EXPECT_EQ(p.getX(), 1);
  EXPECT_EQ(p.getY(), 2);
  EXPECT_EQ(p.getZ(), 0);
  EXPECT_EQ(Punto<int>(1, 2, 3).getZ(), 3);
  EXPECT_EQ(Punto<int>(), Punto<int>(0, 0)); // por defecto, el origen
}

TEST(Punto, Copia) {
  Punto<int> p(1, 2, 3);
  Punto<int> c(p);
  Punto<int> a;
  a = p;
  EXPECT_EQ(c, p);
  EXPECT_EQ(a, p);
}

TEST(Punto, Distancia) {
  EXPECT_DOUBLE_EQ(Punto<int>(0, 0).distancia(Punto<int>(3, 4)), 5.0);
  EXPECT_DOUBLE_EQ(Punto<int>(0, 0).distancia(Punto<int>(1, 1)),
                   std::sqrt(2.0)); // no trunca con T entero
}

TEST(Punto, Igualdad) {
  EXPECT_EQ(Punto<int>(1, 2), Punto<int>(1, 2));
  EXPECT_NE(Punto<int>(1, 2), Punto<int>(2, 1));
  EXPECT_EQ(Punto<double>(0.1 + 0.2, 0), Punto<double>(0.3, 0)); // tolerante
}

TEST(Punto, Salida) {
  std::ostringstream os;
  os << Punto<int>(1, 2, 3);
  EXPECT_EQ(os.str(), "(1,2)"); // z no se imprime
}

TEST(Punto, OrdenLexicografico) {
  EXPECT_TRUE(Punto<int>(0, 5) < Punto<int>(1, 0));  // decide x
  EXPECT_TRUE(Punto<int>(1, 0) < Punto<int>(1, 2));  // a igual x, decide y
  EXPECT_FALSE(Punto<int>(1, 2) < Punto<int>(1, 2)); // orden estricto
  EXPECT_TRUE(Punto<double>(0.3, 0) < Punto<double>(0.1 + 0.2, 0)); // exacto
}

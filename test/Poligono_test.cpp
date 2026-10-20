/**
 * @file Poligono_test.cpp
 * @brief Tests de la clase Poligono.
 */

#include <sstream>
#include <vector>

#include <gtest/gtest.h>

#include "Poligono.hpp"

using P = Punto<int>;

// Cuadrado unitario, recorrido en CCW y en CW.
static const std::vector<P> cuadradoCCW = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
static const std::vector<P> cuadradoCW = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};

TEST(Poligono, CantidadPuntos) {
  EXPECT_EQ(Poligono<int>(cuadradoCCW).cantidadPuntos(), 4);
}

TEST(Poligono, OperadorIndice) {
  Poligono<int> p(cuadradoCCW);
  EXPECT_EQ(p[2], P(1, 1));
  p[2] = P(2, 2); // la referencia permite modificar
  EXPECT_EQ(p[2], P(2, 2));
  const Poligono<int> c(cuadradoCCW);
  EXPECT_EQ(c[1], P(1, 0)); // acceso const
}

TEST(Poligono, EsCCW) {
  EXPECT_TRUE(Poligono<int>(cuadradoCCW).esCCW());
  EXPECT_FALSE(Poligono<int>(cuadradoCW).esCCW());
}

TEST(Poligono, HacerCCW) {
  Poligono<int> p(cuadradoCW);
  p.hacerCCW();
  EXPECT_TRUE(p.esCCW());
  EXPECT_EQ(p[0], P(1, 0)); // orden invertido
  Poligono<int> q(cuadradoCCW);
  q.hacerCCW();
  EXPECT_EQ(q[0], P(0, 0)); // ya era CCW: no cambia
}

TEST(Poligono, HacerCCWEnLongDouble) {
  // Los vertices solo difieren despues del bit 53: en double el area es 0 y
  // hacerCCW no daria vuelta el triangulo, que esta en CW.
  const long double e = 5e-17L;
  Poligono<long double> p({{1, 0}, {1, e}, {1 + e, 0}});
  EXPECT_FALSE(p.esCCW());
  p.hacerCCW();
  EXPECT_TRUE(p.esCCW());
  EXPECT_GT(p.area(), 0);
}

TEST(Poligono, Area) {
  EXPECT_DOUBLE_EQ(Poligono<int>({{0, 0}, {4, 0}, {0, 3}}).area(), 6.0);
  EXPECT_DOUBLE_EQ(Poligono<int>(cuadradoCW).area(), 1.0); // sin signo
}

TEST(Poligono, Salida) {
  std::ostringstream os;
  os << Poligono<int>(cuadradoCCW);
  EXPECT_EQ(os.str(), "[(0,0),(1,0),(1,1),(0,1)]");
}

TEST(Poligono, ConstructorVacio) {
  EXPECT_EQ(Poligono<int>().cantidadPuntos(), 0);
}

TEST(Poligono, Normalizar) {
  Poligono<int> p({{1, 1}, {1, 0}, {0, 0}, {0, 1}}); // CW, empieza en (1,1)
  p.normalizar();
  EXPECT_EQ(p, Poligono<int>(cuadradoCCW)); // CCW y desde (0,0)
  Poligono<int> vacio;
  vacio.normalizar(); // no falla sin vertices
  EXPECT_EQ(vacio.cantidadPuntos(), 0);
}

TEST(Poligono, Igualdad) {
  EXPECT_EQ(Poligono<int>(cuadradoCCW), Poligono<int>(cuadradoCCW));
  EXPECT_NE(Poligono<int>(cuadradoCCW), Poligono<int>(cuadradoCW)); // otro orden
  EXPECT_NE(Poligono<int>(cuadradoCCW), Poligono<int>({{0, 0}, {1, 0}}));
}

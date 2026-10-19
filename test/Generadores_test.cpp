/**
 * @file Generadores_test.cpp
 * @brief Tests de los generadores de puntos.
 */

#include <cmath>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include "GeneradorPuntos.hpp"
#include "IncrementalAlgorithm.hpp"

/// Tamano de la cerradura de una nube.
template <typename T> static int tamanoCerradura(const std::vector<Punto<T>> &nube) {
  return IncrementalAlgorithm<T>().apply(nube).cantidadPuntos();
}

TEST(Generadores, Uniforme) {
  std::mt19937_64 rng(1);
  GeneradorUniforme<double> g;
  const auto nube = g.generar(5000, rng);
  ASSERT_EQ(nube.size(), 5000u);
  for (const auto &q : nube) {
    EXPECT_LE(std::fabs(q.getX()), 1e8);
    EXPECT_LE(std::fabs(q.getY()), 1e8);
  }
  EXPECT_EQ(g.nombre(), "uniforme");
}

TEST(Generadores, CircunferenciaPorcentaje) {
  std::mt19937_64 rng(2);
  EXPECT_EQ(tamanoCerradura(GeneradorCircunferencia<double>(0.1).generar(1000, rng)), 100);
  EXPECT_EQ(tamanoCerradura(GeneradorCircunferencia<double>(1.0).generar(1000, rng)), 1000);
  EXPECT_EQ(tamanoCerradura(GeneradorCircunferencia<double>(0.0).generar(1000, rng)), 3);
  EXPECT_EQ(tamanoCerradura(GeneradorCircunferencia<int>(0.1).generar(1000, rng)), 100);
  EXPECT_EQ(GeneradorCircunferencia<int>(0.1).generar(10, rng).size(), 10u);
}

TEST(Generadores, MismaFiguraEnIntYDouble) {
  std::mt19937_64 a(5), b(5);
  const auto reales = GeneradorCircunferencia<double>(0.2).generar(100, a);
  const auto enteros = GeneradorCircunferencia<int>(0.2).generar(100, b);
  for (std::size_t i = 0; i < reales.size(); ++i) {
    EXPECT_EQ(enteros[i].getX(), std::llround(reales[i].getX()));
    EXPECT_EQ(enteros[i].getY(), std::llround(reales[i].getY()));
  }
}

TEST(Generadores, CuadradoColineal) {
  std::mt19937_64 rng(3);
  const auto nube = GeneradorCuadrado<int>(0.3, 0.0).generar(1000, rng);
  int enBorde = 0;
  for (const auto &q : nube) {
    enBorde += std::abs(q.getX()) == 100000000 || std::abs(q.getY()) == 100000000;
  }
  EXPECT_EQ(enBorde, 300);
  const Poligono<int> c = IncrementalAlgorithm<int>().apply(nube);
  EXPECT_EQ(c, Poligono<int>({{-100000000, -100000000},
                              {100000000, -100000000},
                              {100000000, 100000000},
                              {-100000000, 100000000}}));
  EXPECT_EQ(GeneradorCuadrado<double>(0.3, 0.5).nombre(), "cuadrado");
}

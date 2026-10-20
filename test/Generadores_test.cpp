/**
 * @file Generadores_test.cpp
 * @brief Tests de los generadores de puntos.
 */

#include <cmath>
#include <cstddef>
#include <random>
#include <type_traits>
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

/// Verifica que gt dibuje la figura de gd, escalada en escala, trasladada en
/// centro y redondeada a lo mas en tolerancia.
template <typename T>
static void compararConDouble(const GeneradorPuntos<double> &gd,
                              const GeneradorPuntos<T> &gt, double escala,
                              double centro, double tolerancia) {
  std::mt19937_64 a(5), b(5);
  const auto reales = gd.generar(500, a);
  const auto enT = gt.generar(500, b);
  ASSERT_EQ(enT.size(), reales.size());
  for (std::size_t i = 0; i < reales.size(); ++i) {
    const double x = static_cast<double>(enT[i].getX()) - centro;
    const double y = static_cast<double>(enT[i].getY()) - centro;
    EXPECT_LE(std::fabs(x - escala * reales[i].getX()), tolerancia) << gt.nombre();
    EXPECT_LE(std::fabs(y - escala * reales[i].getY()), tolerancia) << gt.nombre();
  }
}

/// Los generadores en todos los tipos de coordenadas.
template <typename T> class GeneradoresTipos : public ::testing::Test {};
using TiposCoordenada =
    ::testing::Types<short, int, unsigned, long long, float, double, long double>;
TYPED_TEST_SUITE(GeneradoresTipos, TiposCoordenada);

TYPED_TEST(GeneradoresTipos, MismaFiguraQueEnDouble) {
  // Si una coordenada no cupiera en T, daria la vuelta y quedaria lejos de la
  // figura. El cuadrado rotado en 45 grados tiene las esquinas mas lejanas.
  using T = TypeParam;
  const double escala = std::is_same_v<T, short> ? 2e-4 : 1;
  const double centro = std::is_unsigned_v<T> ? 141421357 : 0;
  double tolerancia = 0; // double y long double: la misma figura, sin redondeo
  if (std::is_integral_v<T>) {
    tolerancia = 0.5 + 1e-6; // redondeo al entero mas cercano
  } else if (std::is_same_v<T, float>) {
    tolerancia = 8; // medio paso de float bajo 2^28
  }
  const double pi = 3.14159265358979323846;
  compararConDouble(GeneradorUniforme<double>(), GeneradorUniforme<T>(), escala,
                    centro, tolerancia);
  compararConDouble(GeneradorCircunferencia<double>(0.2),
                    GeneradorCircunferencia<T>(0.2), escala, centro, tolerancia);
  compararConDouble(GeneradorCuadrado<double>(0.3, pi / 4),
                    GeneradorCuadrado<T>(0.3, pi / 4), escala, centro, tolerancia);
}

TEST(Generadores, CircunferenciaShortYUnsigned) {
  std::mt19937_64 rng(2);
  // Con radio 2·10^4 el redondeo saca de la posicion convexa a parte de los
  // puntos de la circunferencia cuando son muchos (de 1000 quedan unos 800);
  // con 100, quedan todos.
  EXPECT_EQ(tamanoCerradura(GeneradorCircunferencia<short>(0.5).generar(200, rng)), 100);
  // unsigned dibuja la figura de int trasladada: el mismo resultado que int.
  EXPECT_EQ(tamanoCerradura(GeneradorCircunferencia<unsigned>(0.5).generar(2000, rng)),
            1000);
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

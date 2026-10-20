/**
 * @file Cerradura_test.cpp
 * @brief Tests de IncrementalAlgorithm y RandomIncrementalAlgorithm.
 */

#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "GeneradorPuntos.hpp"
#include "IncrementalAlgorithm.hpp"
#include "RandomIncrementalAlgorithm.hpp"

/// Igualdad exacta, vertice a vertice con mismoPunto. Poligono::operator== es
/// tolerante en punto flotante y no distingue 1 de 1 + 1e-17 en long double.
template <typename T>
static bool mismaCerradura(const Poligono<T> &a, const Poligono<T> &b) {
  if (a.cantidadPuntos() != b.cantidadPuntos()) {
    return false;
  }
  for (int i = 0; i < a.cantidadPuntos(); ++i) {
    if (!mismoPunto(a[i], b[i])) {
      return false;
    }
  }
  return true;
}

/// Verifica que ambas estrategias, el aleatorio con 20 semillas, den la
/// cerradura esperada (ya normalizada: CCW y desde el menor vertice).
template <typename T>
static void esperarCerradura(const std::vector<Punto<T>> &nube,
                             const std::vector<Punto<T>> &esperada) {
  const Poligono<T> objetivo(esperada);
  IncrementalAlgorithm<T> incremental;
  const Poligono<T> c = incremental.apply(nube);
  EXPECT_TRUE(mismaCerradura(c, objetivo)) << "incremental: " << c;
  for (std::uint64_t semilla = 1; semilla <= 20; ++semilla) {
    RandomIncrementalAlgorithm<T> aleatorio(semilla);
    const Poligono<T> d = aleatorio.apply(nube);
    EXPECT_TRUE(mismaCerradura(d, objetivo)) << "semilla " << semilla << ": " << d;
  }
}

/// Certificado independiente de los algoritmos: la cerradura es estrictamente
/// convexa en CCW y ningun punto de la nube queda a su derecha. O(n h).
template <typename T>
static bool esCerraduraDe(const Poligono<T> &c, const std::vector<Punto<T>> &nube) {
  const int h = c.cantidadPuntos();
  for (int i = 0; i < h; ++i) {
    const Punto<T> &a = c[i], &b = c[(i + 1) % h];
    if (orientacion(a, b, c[(i + 2) % h]) <= 0) {
      return false;
    }
    for (const Punto<T> &q : nube) {
      if (orientacion(a, b, q) < 0) {
        return false;
      }
    }
  }
  return true;
}

/// Compara ambas estrategias entre si y contra el certificado en una nube.
template <typename T>
static void compararEn(const std::vector<Punto<T>> &nube, std::uint64_t semilla,
                       const std::string &caso) {
  IncrementalAlgorithm<T> incremental;
  RandomIncrementalAlgorithm<T> aleatorio(semilla);
  const Poligono<T> a = incremental.apply(nube);
  const Poligono<T> b = aleatorio.apply(nube);
  EXPECT_EQ(a, b) << caso << ", semilla " << semilla;
  EXPECT_TRUE(esCerraduraDe(a, nube)) << caso;
}

/// Los casos con coordenadas enteras no negativas, en todos los tipos.
template <typename T> class CerraduraTipos : public ::testing::Test {};
using TiposCoordenada =
    ::testing::Types<short, int, unsigned, long long, float, double, long double>;
TYPED_TEST_SUITE(CerraduraTipos, TiposCoordenada);

TYPED_TEST(CerraduraTipos, Triangulo) {
  esperarCerradura<TypeParam>({{0, 4}, {4, 0}, {0, 0}}, {{0, 0}, {4, 0}, {0, 4}});
}

TYPED_TEST(CerraduraTipos, PuntosInteriores) {
  esperarCerradura<TypeParam>({{2, 2}, {4, 4}, {0, 0}, {4, 0}, {1, 3}, {0, 4}},
                              {{0, 0}, {4, 0}, {4, 4}, {0, 4}});
  // El ejemplo del upper hull: C = (2,1) queda dentro.
  esperarCerradura<TypeParam>({{0, 0}, {1, 2}, {2, 1}, {3, 3}, {4, 0}},
                              {{0, 0}, {4, 0}, {3, 3}, {1, 2}});
}

TYPED_TEST(CerraduraTipos, ColinealesYRepetidos) {
  // Puntos sobre los lados y esquinas repetidas: solo quedan las esquinas.
  esperarCerradura<TypeParam>({{0, 0}, {2, 0}, {4, 0}, {4, 2}, {4, 4}, {2, 4},
                               {0, 4}, {0, 2}, {0, 0}, {4, 4}, {2, 2}},
                              {{0, 0}, {4, 0}, {4, 4}, {0, 4}});
  // (4,0) en la prolongacion de la arista (0,0) -> (2,0): (2,0) deja de ser vertice.
  esperarCerradura<TypeParam>({{0, 0}, {2, 0}, {1, 3}, {4, 0}},
                              {{0, 0}, {4, 0}, {1, 3}});
  // Repetidos al inicio: el triangulo inicial los salta.
  esperarCerradura<TypeParam>({{1, 1}, {1, 1}, {1, 1}, {0, 0}, {2, 0}, {1, 5}},
                              {{0, 0}, {2, 0}, {1, 5}});
}

TYPED_TEST(CerraduraTipos, Degenerada) {
  esperarCerradura<TypeParam>({}, {});
  esperarCerradura<TypeParam>({{3, 1}}, {{3, 1}});
  esperarCerradura<TypeParam>({{5, 5}, {5, 5}, {5, 5}}, {{5, 5}});
  esperarCerradura<TypeParam>({{2, 0}, {1, 0}}, {{1, 0}, {2, 0}});
  esperarCerradura<TypeParam>({{1, 1}, {3, 3}, {2, 2}, {0, 0}, {3, 3}},
                              {{0, 0}, {3, 3}}); // todos colineales: un segmento
}

TYPED_TEST(CerraduraTipos, AmbasCoinciden) {
  using T = TypeParam;
  GeneradorUniforme<T> uniforme;
  GeneradorCircunferencia<T> circunferencia(0.5);
  GeneradorCuadrado<T> cuadrado(0.3, 0.0);
  const GeneradorPuntos<T> *generadores[] = {&uniforme, &circunferencia, &cuadrado};
  for (const GeneradorPuntos<T> *g : generadores) {
    for (std::uint64_t semilla = 1; semilla <= 5; ++semilla) {
      std::mt19937_64 rng(semilla);
      compararEn(g->generar(2000, rng), semilla, g->nombre());
    }
  }
}

TEST(Cerradura, TrianguloConDecimales) {
  esperarCerradura<double>({{0.5, 0.25}, {-1.5, 2}, {3, -0.75}},
                           {{-1.5, 2}, {0.5, 0.25}, {3, -0.75}});
  esperarCerradura<float>({{0.5f, 0.25f}, {-1.5f, 2}, {3, -0.75f}},
                          {{-1.5f, 2}, {0.5f, 0.25f}, {3, -0.75f}});
  esperarCerradura<long double>({{0.5L, 0.25L}, {-1.5L, 2}, {3, -0.75L}},
                                {{-1.5L, 2}, {0.5L, 0.25L}, {3, -0.75L}});
}

TEST(Cerradura, LongDoubleCasiRepetidos) {
  // Con Ancho<long double> = double, A y A2 se confundian en orientacion y,
  // con la semilla 2, el aleatorio devolvia el segmento (0,-1) - A2.
  const Punto<long double> a(1, 0), a2(1 + 1e-17L, 0), b(0, 1), c(0, -1);
  esperarCerradura<long double>({a, a2, b, c}, {c, a2, b});
}

TEST(Cerradura, LongDoubleTrianguloFino) {
  // En double los tres vertices son casi el mismo punto y el area da 0, asi
  // que normalizar no daba vuelta la cadena en CW del incremental.
  const long double e = 5e-17L;
  esperarCerradura<long double>({{1, e}, {1 + e, 0}, {1, 0}},
                                {{1, 0}, {1 + e, 0}, {1, e}});
}

TEST(Cerradura, LongLongCoordenadasGrandes) {
  // En long long el producto cruz de estas coordenadas desbordaria.
  for (int bits : {40, 59}) {
    std::uniform_int_distribution<long long> d(-(1LL << bits), 1LL << bits);
    for (std::uint64_t semilla = 1; semilla <= 5; ++semilla) {
      std::mt19937_64 rng(semilla);
      std::vector<Punto<long long>> nube;
      for (int i = 0; i < 2000; ++i) {
        const long long x = d(rng);
        nube.emplace_back(x, d(rng));
      }
      compararEn(nube, semilla, "2^" + std::to_string(bits));
    }
  }
}

TEST(Cerradura, PorLaInterfaz) {
  std::vector<std::unique_ptr<EstrategiaCerradura<int>>> estrategias;
  estrategias.push_back(std::make_unique<IncrementalAlgorithm<int>>());
  estrategias.push_back(std::make_unique<RandomIncrementalAlgorithm<int>>(7));
  EXPECT_EQ(estrategias[0]->nombre(), "incremental");
  EXPECT_EQ(estrategias[1]->nombre(), "random_incremental");
  for (auto &e : estrategias) {
    EXPECT_EQ(e->apply({{0, 0}, {2, 2}, {2, 0}, {1, 1}}),
              Poligono<int>({{0, 0}, {2, 0}, {2, 2}}));
  }
}

TEST(Cerradura, Reasignaciones) {
  RandomIncrementalAlgorithm<double> aleatorio(3);
  aleatorio.apply({{0, 0}, {1, 0}, {0, 1}});
  EXPECT_EQ(aleatorio.getReasignaciones(), 0u); // solo el triangulo inicial

  std::mt19937_64 rng(1);
  const auto nube = GeneradorCircunferencia<double>(1.0).generar(1000, rng);
  aleatorio.apply(nube);
  EXPECT_GT(aleatorio.getReasignaciones(), 1000u); // todos en la cerradura
  EXPECT_EQ(aleatorio.apply(nube).cantidadPuntos(), 1000);
}

/**
 * @file Cerradura_test.cpp
 * @brief Tests de IncrementalAlgorithm y RandomIncrementalAlgorithm.
 */

#include <cstdint>
#include <memory>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include "GeneradorPuntos.hpp"
#include "IncrementalAlgorithm.hpp"
#include "RandomIncrementalAlgorithm.hpp"

/// Verifica que ambas estrategias, el aleatorio con 20 semillas, den la
/// cerradura esperada (ya normalizada: CCW y desde el menor vertice).
template <typename T>
static void esperarCerradura(const std::vector<Punto<T>> &nube,
                             const std::vector<Punto<T>> &esperada) {
  const Poligono<T> objetivo(esperada);
  IncrementalAlgorithm<T> incremental;
  EXPECT_EQ(incremental.apply(nube), objetivo) << "incremental";
  for (std::uint64_t semilla = 1; semilla <= 20; ++semilla) {
    RandomIncrementalAlgorithm<T> aleatorio(semilla);
    EXPECT_EQ(aleatorio.apply(nube), objetivo) << "semilla " << semilla;
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

TEST(Cerradura, Triangulo) {
  esperarCerradura<int>({{0, 4}, {4, 0}, {0, 0}}, {{0, 0}, {4, 0}, {0, 4}});
  esperarCerradura<double>({{0.5, 0.25}, {-1.5, 2}, {3, -0.75}},
                           {{-1.5, 2}, {0.5, 0.25}, {3, -0.75}});
}

TEST(Cerradura, PuntosInteriores) {
  esperarCerradura<int>({{2, 2}, {4, 4}, {0, 0}, {4, 0}, {1, 3}, {0, 4}},
                        {{0, 0}, {4, 0}, {4, 4}, {0, 4}});
  // El ejemplo del upper hull: C = (2,1) queda dentro.
  esperarCerradura<int>({{0, 0}, {1, 2}, {2, 1}, {3, 3}, {4, 0}},
                        {{0, 0}, {4, 0}, {3, 3}, {1, 2}});
}

TEST(Cerradura, ColinealesYRepetidos) {
  // Puntos sobre los lados y esquinas repetidas: solo quedan las esquinas.
  esperarCerradura<int>({{0, 0}, {2, 0}, {4, 0}, {4, 2}, {4, 4}, {2, 4}, {0, 4},
                         {0, 2}, {0, 0}, {4, 4}, {2, 2}},
                        {{0, 0}, {4, 0}, {4, 4}, {0, 4}});
  // (4,0) en la prolongacion de la arista (0,0) -> (2,0): (2,0) deja de ser vertice.
  esperarCerradura<int>({{0, 0}, {2, 0}, {1, 3}, {4, 0}},
                        {{0, 0}, {4, 0}, {1, 3}});
  // Repetidos al inicio: el triangulo inicial los salta.
  esperarCerradura<int>({{1, 1}, {1, 1}, {1, 1}, {0, 0}, {2, 0}, {1, 5}},
                        {{0, 0}, {2, 0}, {1, 5}});
}

TEST(Cerradura, Degenerada) {
  esperarCerradura<int>({}, {});
  esperarCerradura<int>({{3, 1}}, {{3, 1}});
  esperarCerradura<int>({{5, 5}, {5, 5}, {5, 5}}, {{5, 5}});
  esperarCerradura<int>({{2, 0}, {1, 0}}, {{1, 0}, {2, 0}});
  esperarCerradura<int>({{1, 1}, {3, 3}, {2, 2}, {0, 0}, {3, 3}},
                        {{0, 0}, {3, 3}}); // todos colineales: un segmento
}

/// Compara ambas estrategias contra el certificado en nubes generadas.
template <typename T> static void compararEnNubes() {
  GeneradorUniforme<T> uniforme;
  GeneradorCircunferencia<T> circunferencia(0.5);
  GeneradorCuadrado<T> cuadrado(0.3, 0.0);
  const GeneradorPuntos<T> *generadores[] = {&uniforme, &circunferencia, &cuadrado};
  for (const GeneradorPuntos<T> *g : generadores) {
    for (std::uint64_t semilla = 1; semilla <= 5; ++semilla) {
      std::mt19937_64 rng(semilla);
      const std::vector<Punto<T>> nube = g->generar(2000, rng);
      IncrementalAlgorithm<T> incremental;
      RandomIncrementalAlgorithm<T> aleatorio(semilla);
      const Poligono<T> a = incremental.apply(nube);
      const Poligono<T> b = aleatorio.apply(nube);
      EXPECT_EQ(a, b) << g->nombre() << ", semilla " << semilla;
      EXPECT_TRUE(esCerraduraDe(a, nube)) << g->nombre();
    }
  }
}

TEST(Cerradura, AmbasCoincidenInt) { compararEnNubes<int>(); }

TEST(Cerradura, AmbasCoincidenDouble) { compararEnNubes<double>(); }

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

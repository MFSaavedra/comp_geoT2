/**
 * @file Geometria_test.cpp
 * @brief Tests de los predicados de Geometria.hpp.
 */

#include <type_traits>

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

/// Los tests de un mismo caso en todos los tipos de coordenadas.
template <typename T> class GeometriaTipos : public ::testing::Test {};
using TiposCoordenada =
    ::testing::Types<short, int, unsigned, long long, float, double, long double>;
TYPED_TEST_SUITE(GeometriaTipos, TiposCoordenada);

TYPED_TEST(GeometriaTipos, Orientacion) {
  using P = Punto<TypeParam>;
  // a -> b va hacia la izquierda: con unsigned, restar en T daria la vuelta.
  const P a(5, 5), b(3, 5);
  EXPECT_EQ(orientacion(a, b, P(4, 1)), 1);
  EXPECT_EQ(orientacion(a, b, P(4, 9)), -1);
  EXPECT_EQ(orientacion(a, b, P(0, 5)), 0);
}

TEST(Geometria, OrientacionShort) {
  // Los productos (~2^32) no caben en short ni en int.
  const Punto<short> a(-32768, -32768), b(32767, -32768);
  EXPECT_EQ(orientacion(a, b, Punto<short>(32767, 32767)), 1);
  EXPECT_EQ(orientacion(a, b, Punto<short>(0, -32768)), 0);
}

TEST(Geometria, OrientacionLongLong) {
  // En long long, 2^32 * 2^32 - 1 daria la vuelta a -1; en Entero128 no.
  const long long t = 1LL << 32;
  const Punto<long long> o(0, 0), b(t, 1), c(1, t);
  EXPECT_EQ(orientacion(o, b, c), 1);
  const long long g = 1LL << 60;
  const Punto<long long> p(-g, -g), q(g, -g);
  EXPECT_EQ(orientacion(p, q, Punto<long long>(g, g)), 1);
  EXPECT_EQ(orientacion(p, q, Punto<long long>(0, -g)), 0);
  EXPECT_EQ(orientacion(p, q, Punto<long long>(0, -g - 1)), -1);
}

TEST(Geometria, OrientacionLongDouble) {
  // 1 + 1e-17 es distinto de 1 en long double, pero no en double.
  const Punto<long double> a(1, 0), a2(1 + 1e-17L, 0), b(0, 1);
  EXPECT_FALSE(mismoPunto(a, a2));
  EXPECT_EQ(orientacion(a, a2, b), 1);
  EXPECT_EQ(orientacion(a, a2, Punto<long double>(0, -1)), -1);
}

TEST(Geometria, SignoCruz) {
  EXPECT_EQ(signoCruz(Vector<long long>(1, 0), Vector<long long>(0, 1)), 1);
  EXPECT_EQ(signoCruz(Vector<long long>(0, 1), Vector<long long>(1, 0)), -1);
  EXPECT_EQ(signoCruz(Vector<long long>(2, 2), Vector<long long>(-1, -1)), 0);
  EXPECT_EQ(signoCruz(Vector<double>(1, 0), Vector<double>(1, 1e-300)), 1);
}

TEST(Geometria, TiposDeCoordenadas) {
  EXPECT_TRUE(esTipoCoordenada<short>);
  EXPECT_TRUE(esTipoCoordenada<int>);
  EXPECT_TRUE(esTipoCoordenada<unsigned>);
  EXPECT_TRUE(esTipoCoordenada<long long>);
  EXPECT_TRUE(esTipoCoordenada<float>);
  EXPECT_TRUE(esTipoCoordenada<double>);
  EXPECT_TRUE(esTipoCoordenada<long double>);
  EXPECT_FALSE(esTipoCoordenada<char>);
  EXPECT_FALSE(esTipoCoordenada<bool>);
  EXPECT_FALSE(esTipoCoordenada<long>);
  EXPECT_FALSE(esTipoCoordenada<unsigned long long>);
}

TEST(Geometria, Ancho) {
  EXPECT_TRUE((std::is_same_v<Ancho<short>, long long>));
  EXPECT_TRUE((std::is_same_v<Ancho<int>, long long>));
  EXPECT_TRUE((std::is_same_v<Ancho<unsigned>, long long>));
  EXPECT_TRUE((std::is_same_v<Ancho<long long>, Entero128>));
  EXPECT_TRUE((std::is_same_v<Ancho<float>, double>));
  EXPECT_TRUE((std::is_same_v<Ancho<double>, double>));
  EXPECT_TRUE((std::is_same_v<Ancho<long double>, long double>));
  EXPECT_EQ(sizeof(Entero128), 16u);
}

TEST(Geometria, MismoPunto) {
  EXPECT_TRUE(mismoPunto(Punto<int>(1, 2), Punto<int>(1, 2)));
  EXPECT_FALSE(mismoPunto(Punto<int>(1, 2), Punto<int>(2, 1)));
  EXPECT_FALSE(mismoPunto(Punto<double>(0.1 + 0.2, 0), Punto<double>(0.3, 0)));
  EXPECT_TRUE(mismoPunto(Punto<int>(1, 2), Punto<int>(1, 2, 7))); // ignora z
}

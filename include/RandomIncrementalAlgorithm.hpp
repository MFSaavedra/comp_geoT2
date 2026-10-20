/**
 * @file RandomIncrementalAlgorithm.hpp
 * @brief Algoritmo incremental aleatorio con listas de conflicto (notas de
 *        15-451 de CMU, seccion 2.2).
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "EstrategiaCerradura.hpp"
#include "Geometria.hpp"

/**
 * @brief Cerradura convexa incremental en orden aleatorio, O(n log n) esperado.
 * @tparam T Tipo de las coordenadas (ver esTipoCoordenada). Con T entero, los
 *         calculos son exactos si en cada eje max - min < 2^29 (T de hasta
 *         32 bits) o < 2^61 (T de 64 bits); ver ladoDelRayo.
 *
 * Parte de un triangulo y agrega los demas puntos en orden aleatorio. Para no
 * buscar en toda la cerradura que aristas ve cada punto nuevo, cada punto que
 * falta por insertar guarda una arista visible: la que cruza el rayo que va
 * desde un punto fijo C (el centroide del triangulo inicial) hasta el punto.
 * Cada arista guarda, a su vez, la "cubeta" con los puntos que la apuntan.
 *
 * Al insertar un punto q se eliminan las aristas que q ve, se agregan dos
 * (l -> q y q -> r) y solo se reasignan los puntos de las cubetas eliminadas.
 * Un punto que queda dentro de la cerradura se descarta para siempre, porque
 * la cerradura solo crece.
 *
 * La cerradura se guarda como lista doblemente enlazada en CCW sobre los
 * indices de la copia barajada; una arista se identifica por su vertice de
 * origen v (la arista v -> sig[v]).
 */
template <typename T>
class RandomIncrementalAlgorithm : public EstrategiaCerradura<T> {
public:
  /**
   * @brief Construye la estrategia.
   * @param semilla Semilla del barajado; la misma semilla da la misma ejecucion.
   */
  explicit RandomIncrementalAlgorithm(std::uint64_t semilla = 1)
      : semilla(semilla) {}

  /**
   * @brief Calcula la cerradura convexa de la nube.
   * @param nube Puntos de entrada.
   * @return La cerradura, con la convencion de EstrategiaCerradura.
   */
  Poligono<T> apply(const std::vector<Punto<T>> &nube) override {
    p = nube;
    std::mt19937_64 rng(semilla);
    std::shuffle(p.begin(), p.end(), rng);
    reasignaciones = 0;

    Poligono<T> cerradura =
        moverTrianguloAlInicio() ? construir() : cerraduraDegenerada();
    liberarMemoria();
    return cerradura;
  }

  /** @brief Nombre de la estrategia. */
  std::string nombre() const override { return "random_incremental"; }

  /**
   * @brief Cantidad de veces que un punto paso a una arista nueva durante la
   *        ultima llamada a apply.
   * @return Las reasignaciones; es el costo que acota el analisis O(n log n).
   */
  std::size_t getReasignaciones() const { return reasignaciones; }

private:
  using W = Ancho<T>;                ///< Tipo de los calculos con C.
  static constexpr int NINGUNA = -1; ///< Indice nulo.

  std::uint64_t semilla;          ///< Semilla del barajado.
  std::size_t reasignaciones = 0; ///< Contador de la ultima llamada a apply.

  // Estado de una llamada a apply; los arreglos se indexan por punto.
  std::vector<Punto<T>> p; ///< Copia barajada de la nube.
  std::vector<int> sig;    ///< Si es vertice: siguiente vertice en CCW.
  std::vector<int> ant;    ///< Si es vertice: vertice anterior en CCW.
  std::vector<int> arista; ///< Si es futuro: origen de la arista que ve, o NINGUNA.
  /// Si es vertice: la cubeta de su arista, con los puntos que la ven.
  std::vector<std::vector<int>> cubeta;
  std::vector<std::vector<int>> eliminadas; ///< Cubetas por vaciar al insertar.
  W sumaX = 0; ///< Coordenada x de 3C, la suma de los vertices iniciales.
  W sumaY = 0; ///< Coordenada y de 3C.

  /**
   * @brief Deja en p[0], p[1], p[2] un triangulo no degenerado, en CCW.
   * @return false si todos los puntos son colineales (o hay menos de 3).
   * @note Toma el primer punto distinto de p[0] y luego el primero que no es
   *       colineal con ambos: con puntos enteros o colineales, los tres
   *       primeros del barajado pueden no servir.
   */
  bool moverTrianguloAlInicio() {
    const std::size_t n = p.size();
    std::size_t j = 1;
    while (j < n && mismoPunto(p[j], p[0])) {
      ++j;
    }
    std::size_t k = j + 1;
    while (k < n && orientacion(p[0], p[j], p[k]) == 0) {
      ++k;
    }
    if (k >= n) {
      return false;
    }
    std::swap(p[1], p[j]);
    std::swap(p[2], p[k]);
    if (orientacion(p[0], p[1], p[2]) < 0) {
      std::swap(p[1], p[2]);
    }
    return true;
  }

  /**
   * @brief Cerradura de puntos todos colineales: el segmento entre los extremos.
   * @return Vacia, un punto o dos puntos en orden lexicografico.
   */
  Poligono<T> cerraduraDegenerada() const {
    if (p.empty()) {
      return Poligono<T>();
    }
    const auto [menor, mayor] = std::minmax_element(p.begin(), p.end());
    if (mismoPunto(*menor, *mayor)) {
      return Poligono<T>({*menor});
    }
    return Poligono<T>({*menor, *mayor});
  }

  /**
   * @brief El algoritmo, una vez que p[0], p[1], p[2] forman un triangulo CCW.
   * @return La cerradura de todos los puntos.
   */
  Poligono<T> construir() {
    const int n = static_cast<int>(p.size());
    sig.assign(n, NINGUNA);
    ant.assign(n, NINGUNA);
    arista.assign(n, NINGUNA);
    cubeta.assign(n, {});

    sig[0] = 1, sig[1] = 2, sig[2] = 0;
    ant[0] = 2, ant[1] = 0, ant[2] = 1;
    sumaX = W(p[0].getX()) + W(p[1].getX()) + W(p[2].getX());
    sumaY = W(p[0].getY()) + W(p[1].getY()) + W(p[2].getY());

    for (int q = 3; q < n; ++q) {
      asignarEnTriangulo(q);
    }

    int ultimo = 0; // un vertice de la cerradura actual
    for (int q = 3; q < n; ++q) {
      if (arista[q] != NINGUNA) { // si es NINGUNA, q quedo dentro
        insertar(q);
        ultimo = q;
      }
    }
    return recorrer(ultimo);
  }

  /**
   * @brief Asocia el punto q a la arista del triangulo inicial que cruza el
   *        rayo C -> q, o lo descarta si queda dentro.
   * @param q Indice del punto.
   * @note El rayo cruza la arista v -> sig[v] cuando q esta en el sector entre
   *       los rayos C -> v (incluido) y C -> sig[v] (excluido). Si q == C no
   *       cae en ningun sector, y esta dentro.
   */
  void asignarEnTriangulo(int q) {
    for (int v = 0; v < 3; ++v) {
      if (ladoDelRayo(v, q) >= 0 && ladoDelRayo(sig[v], q) < 0) {
        asignar(q, v);
        return;
      }
    }
  }

  /**
   * @brief Inserta el punto q, que esta fuera de la cerradura actual.
   * @param q Indice del punto; arista[q] es una arista que q ve.
   */
  void insertar(int q) {
    // 1. Cadena de aristas visibles, desde arista[q] hacia ambos lados. Una
    //    arista cuya recta pasa por q tambien se elimina: q queda en su
    //    prolongacion, y el vertice que comparten deja de ser extremo. Los
    //    topes (ant[l] != fin, r != l) solo actuan con aritmetica inexacta.
    const int inicio = arista[q];
    const int fin = sig[inicio];
    int l = inicio;
    while (ant[l] != fin && orientacion(p[ant[l]], p[l], p[q]) <= 0) {
      l = ant[l];
    }
    int r = fin;
    while (r != l && orientacion(p[r], p[sig[r]], p[q]) <= 0) {
      r = sig[r];
    }

    // 2. Sacar las cubetas de las aristas eliminadas, de l hasta r.
    eliminadas.clear();
    for (int v = l; v != r; v = sig[v]) {
      eliminadas.push_back(std::move(cubeta[v]));
      cubeta[v].clear();
    }

    // 3. Reemplazar la cadena l .. r por l -> q -> r.
    sig[l] = q, ant[q] = l;
    sig[q] = r, ant[r] = q;
    arista[q] = NINGUNA;

    // 4. Cada punto de esas cubetas cruza ahora l -> q o q -> r: lo decide de
    //    que lado del rayo C -> q esta.
    for (const std::vector<int> &vieja : eliminadas) {
      for (int f : vieja) {
        if (f != q) {
          ++reasignaciones;
          asignar(f, ladoDelRayo(q, f) < 0 ? l : q);
        }
      }
    }
  }

  /**
   * @brief Pone el punto f en la cubeta de la arista v -> sig[v] si la ve; si
   *        no la ve, f esta dentro de la cerradura y se descarta.
   * @param f Indice del punto.
   * @param v Origen de la arista que cruza el rayo C -> f.
   */
  void asignar(int f, int v) {
    if (orientacion(p[v], p[sig[v]], p[f]) < 0) {
      arista[f] = v;
      cubeta[v].push_back(f);
    } else {
      arista[f] = NINGUNA;
    }
  }

  /**
   * @brief Orientacion de (C, p[v], p[f]), sin calcular C.
   * @param v Indice del punto que define el rayo C -> p[v].
   * @param f Indice del punto a clasificar.
   * @return 1 si p[f] esta a la izquierda del rayo, -1 a la derecha, 0 sobre
   *         su recta.
   * @note C = suma / 3 no tiene coordenadas enteras; se usa 3p - suma, que
   *       es 3 veces el vector C -> p y tiene la misma orientacion. Si en un
   *       eje max - min = L, entonces |3p - suma| < 3L y el producto cruz es
   *       menor que 18 L^2: cabe en long long si L < 2^29 y en Entero128 si
   *       L < 2^61. Usa signoCruz y no orientacion porque los vectores ya
   *       estan en W, y orientacion los volveria a promover.
   */
  int ladoDelRayo(int v, int f) const {
    const Vector<W> cv(3 * W(p[v].getX()) - sumaX, 3 * W(p[v].getY()) - sumaY);
    const Vector<W> cf(3 * W(p[f].getX()) - sumaX, 3 * W(p[f].getY()) - sumaY);
    return signoCruz(cv, cf);
  }

  /**
   * @brief Recorre la cerradura final desde un vertice.
   * @param desde Indice de un vertice de la cerradura.
   * @return La cerradura normalizada.
   */
  Poligono<T> recorrer(int desde) const {
    std::vector<Punto<T>> vertices;
    int v = desde;
    do {
      vertices.push_back(p[v]);
      v = sig[v];
    } while (v != desde);
    Poligono<T> cerradura(vertices);
    cerradura.normalizar();
    return cerradura;
  }

  /**
   * @brief Libera los arreglos de trabajo.
   * @note Asi cada llamada a apply reserva su memoria igual que la primera,
   *       como IncrementalAlgorithm, y las mediciones repetidas son comparables.
   */
  void liberarMemoria() {
    std::vector<Punto<T>>().swap(p);
    std::vector<int>().swap(sig);
    std::vector<int>().swap(ant);
    std::vector<int>().swap(arista);
    std::vector<std::vector<int>>().swap(cubeta);
    std::vector<std::vector<int>>().swap(eliminadas);
  }
};

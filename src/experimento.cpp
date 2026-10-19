/**
 * @file experimento.cpp
 * @brief Experimento completo de ANALISIS.pdf; escribe un archivo CSV.
 *
 * Uso: experimento [archivo.csv]   (por defecto resultados/datos.csv)
 *
 * Hay cuatro partes, cada una con int y con double:
 * - escalamiento: tiempo vs n, con puntos uniformes y con todos en la cerradura;
 * - porcentaje: tiempo vs porcentaje de puntos en la cerradura, para tres n;
 * - colinealidad: cerraduras con y sin puntos colineales;
 * - varianza: una misma nube con 20 semillas del algoritmo aleatorio.
 *
 * Cada caso genera una nube, mide ambas estrategias sobre ella y escribe una
 * fila por estrategia. Las semillas son fijas: las operaciones contadas (h,
 * reasignaciones) se reproducen exactamente; los tiempos dependen de la maquina.
 */

#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "GeneradorPuntos.hpp"
#include "IncrementalAlgorithm.hpp"
#include "Medicion.hpp"
#include "RandomIncrementalAlgorithm.hpp"

/// Instancias (nubes distintas) por caso.
static const int INSTANCIAS = 10;

/**
 * @brief Mide ambas estrategias sobre una nube y escribe sus dos filas.
 * @tparam T Tipo de las coordenadas.
 * @param csv Archivo de salida.
 * @param experimento Parte del experimento.
 * @param tipo Nombre del tipo de las coordenadas.
 * @param caso Nombre del generador con sus parametros.
 * @param parametro Porcentaje en la cerradura (o en el borde del cuadrado).
 * @param g Generador de la nube.
 * @param n Cantidad de puntos.
 * @param instancia Numero de instancia; fija la semilla de la nube.
 * @param semilla Semilla del algoritmo aleatorio.
 */
template <typename T>
void medirCaso(std::ostream &csv, const std::string &experimento,
               const std::string &tipo, const std::string &caso, double parametro,
               const GeneradorPuntos<T> &g, std::size_t n, int instancia,
               std::uint64_t semilla) {
  std::mt19937_64 rng(1000003ULL * instancia + n); // misma nube en int y double
  const std::vector<Punto<T>> nube = g.generar(n, rng);

  IncrementalAlgorithm<T> incremental;
  RandomIncrementalAlgorithm<T> aleatorio(semilla);
  Poligono<T> a, b;
  double tA, tB;
  if (instancia % 2 == 0) { // alterna el orden, por si el primero sale perjudicado
    tA = medirMs([&] { a = incremental.apply(nube); });
    tB = medirMs([&] { b = aleatorio.apply(nube); });
  } else {
    tB = medirMs([&] { b = aleatorio.apply(nube); });
    tA = medirMs([&] { a = incremental.apply(nube); });
  }

  const int iguales = (a == b) ? 1 : 0;
  const std::string comun = experimento + "," + tipo + "," + caso + "," +
                            std::to_string(parametro) + "," + std::to_string(n) +
                            "," + std::to_string(instancia) + "," +
                            std::to_string(semilla) + ",";
  csv << comun << incremental.nombre() << "," << tA << "," << a.cantidadPuntos()
      << ",0," << iguales << "\n";
  csv << comun << aleatorio.nombre() << "," << tB << "," << b.cantidadPuntos()
      << "," << aleatorio.getReasignaciones() << "," << iguales << "\n";
  csv.flush();
}

/**
 * @brief Corre las cuatro partes del experimento con coordenadas de tipo T.
 * @tparam T Tipo de las coordenadas.
 * @param csv Archivo de salida.
 * @param tipo Nombre del tipo de las coordenadas.
 */
template <typename T> void correrTodo(std::ostream &csv, const std::string &tipo) {
  const std::vector<std::size_t> tamanos = {10000,  20000,  50000,  100000,
                                            200000, 500000, 1000000};
  const std::vector<std::size_t> tresTamanos = {10000, 100000, 1000000};
  const std::vector<double> porcentajes = {0.001, 0.002, 0.005, 0.01, 0.02,
                                           0.05,  0.1,   0.2,   0.5,  1.0};

  std::cerr << tipo << ": escalamiento\n";
  GeneradorUniforme<T> uniforme;
  GeneradorCircunferencia<T> todos(1.0);
  for (std::size_t n : tamanos) {
    for (int i = 0; i < INSTANCIAS; ++i) {
      medirCaso(csv, "escalamiento", tipo, "uniforme", 0, uniforme, n, i, i + 1);
      medirCaso(csv, "escalamiento", tipo, "circunferencia", 1, todos, n, i, i + 1);
    }
  }

  for (double p : porcentajes) {
    std::cerr << tipo << ": porcentaje " << p << "\n";
    GeneradorCircunferencia<T> g(p);
    for (std::size_t n : tresTamanos) {
      for (int i = 0; i < INSTANCIAS; ++i) {
        medirCaso(csv, "porcentaje", tipo, "circunferencia", p, g, n, i, i + 1);
      }
    }
  }

  std::cerr << tipo << ": colinealidad\n";
  const double treintaGrados = 3.14159265358979323846 / 6;
  GeneradorCuadrado<T> recto(0.1, 0.0), rotado(0.1, treintaGrados);
  GeneradorCircunferencia<T> circunferencia(0.1);
  for (std::size_t n : tresTamanos) {
    for (int i = 0; i < INSTANCIAS; ++i) {
      medirCaso(csv, "colinealidad", tipo, "cuadrado_0", 0.1, recto, n, i, i + 1);
      medirCaso(csv, "colinealidad", tipo, "cuadrado_30", 0.1, rotado, n, i, i + 1);
      medirCaso(csv, "colinealidad", tipo, "circunferencia", 0.1, circunferencia,
                n, i, i + 1);
    }
  }

  std::cerr << tipo << ": varianza\n";
  for (std::uint64_t s = 1; s <= 20; ++s) {
    medirCaso(csv, "varianza", tipo, "uniforme", 0, uniforme, 1000000, 0, s);
    medirCaso(csv, "varianza", tipo, "circunferencia", 1, todos, 1000000, 0, s);
  }
}

/**
 * @brief Corre el experimento completo.
 * @param argc Cantidad de argumentos.
 * @param argv argv[1], opcional: archivo CSV de salida.
 * @return 0 si se pudo escribir el archivo.
 */
int main(int argc, char **argv) {
  const std::string ruta = argc > 1 ? argv[1] : "resultados/datos.csv";
  std::ofstream csv(ruta);
  if (!csv) {
    std::cerr << "no se pudo abrir " << ruta << "\n";
    return 1;
  }
  csv << "experimento,tipo,generador,parametro,n,instancia,semilla,algoritmo,"
         "tiempo_ms,h,reasignaciones,iguales\n";
  correrTodo<double>(csv, "double");
  correrTodo<int>(csv, "int");
  std::cerr << "listo: " << ruta << "\n";
  return 0;
}

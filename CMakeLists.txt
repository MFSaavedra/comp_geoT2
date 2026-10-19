cmake_minimum_required(VERSION 3.14)
project(comp_geoT2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# build/compile_commands.json, para que clangd encuentre include/.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Las mediciones de tiempo solo tienen sentido con optimizacion: sin tipo de
# build, CMake compila sin -O. Release (-O3 -DNDEBUG) es el valor por defecto.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de build" FORCE)
endif()

enable_testing()

# ----------------------------------------------------------------------------
# Core: Punto / Vector / Poligono, las estrategias de cerradura y los
# generadores. Todo es template, asi que el codigo vive en los headers y no hay
# nada que compilar, de ahi INTERFACE. Lo que enlace `geo` recibe include/.
# ----------------------------------------------------------------------------
add_library(geo INTERFACE)
target_include_directories(geo INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(geo INTERFACE cxx_std_17)

# ----------------------------------------------------------------------------
# Programas: main (ejemplo y comparacion rapida) y experimento (CSV completo).
# ----------------------------------------------------------------------------
foreach(programa main experimento)
  add_executable(${programa} src/${programa}.cpp)
  target_link_libraries(${programa} PRIVATE geo)
  target_compile_options(${programa} PRIVATE -Wall -Wextra)
endforeach()

# ----------------------------------------------------------------------------
# Tests. Prefer the system GTest (Arch's `gtest` package ships the CMake config
# in /usr/lib/cmake/GTest); fall back to building it from upstream when absent.
# ----------------------------------------------------------------------------
file(GLOB GEO_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

if(GEO_TEST_SOURCES)
  find_package(GTest CONFIG QUIET)

  if(NOT GTest_FOUND)
    message(STATUS "GTest not found locally, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
        )
    FetchContent_MakeAvailable(googletest)
  endif()

  include(GoogleTest)

  # One binary for the whole suite; gtest_main supplies main(). CONFIGURE_DEPENDS
  # above re-globs on build, so a new test/*.cpp is picked up without editing this.
  add_executable(geo_tests ${GEO_TEST_SOURCES})
  target_link_libraries(geo_tests PRIVATE geo GTest::gtest_main)
  gtest_discover_tests(geo_tests)
endif()

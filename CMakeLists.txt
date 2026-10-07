cmake_minimum_required(VERSION 3.20)
project(PHOTON_APEX_VELOX_GATE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(PHOTON_ENABLE_AVX512 "Enable AVX-512 code generation" ON)
option(PHOTON_ENABLE_WARNINGS "Enable strict compiler warnings" ON)

add_library(photon_core STATIC
  src/ouch5_gateway.cpp
  src/avx512_signal_engine.cpp
)
target_include_directories(photon_core PUBLIC include)
target_compile_options(photon_core PRIVATE -O3 -march=native)
if(PHOTON_ENABLE_AVX512)
  target_compile_options(photon_core PRIVATE -mavx512f)
endif()
if(PHOTON_ENABLE_WARNINGS)
  target_compile_options(photon_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

add_executable(photon_smoke tests/smoke.cpp)
target_link_libraries(photon_smoke PRIVATE photon_core)

add_executable(photon_hotpath_bench tools/benchmark_hot_path.cpp)
target_link_libraries(photon_hotpath_bench PRIVATE photon_core)

enable_testing()
add_test(NAME photon_smoke COMMAND photon_smoke)

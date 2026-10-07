cmake_minimum_required(VERSION 3.20)
project(PHOTON_APEX_VELOX_GATE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(PHOTON_ENABLE_AVX512 "Enable AVX-512 code generation" ON)
option(PHOTON_ENABLE_WARNINGS "Enable strict compiler warnings" ON)

add_library(photon_core STATIC
  src/ouch5_gateway.cpp
  src/avx512_signal_engine.cpp
  src/market_intelligence.cpp
  src/scenario_engine.cpp
  sandbox/automated_research.cpp
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

add_executable(photon_market_intelligence_test tests/test_market_intelligence.cpp)
target_link_libraries(photon_market_intelligence_test PRIVATE photon_core)

enable_testing()
add_test(NAME photon_smoke COMMAND photon_smoke)
add_test(NAME photon_market_intelligence COMMAND photon_market_intelligence_test)
add_executable(photon_sandbox_test tests/test_sandbox.cpp sandbox/replay_engine.cpp)
 target_include_directories(photon_sandbox_test PRIVATE sandbox)
 target_link_libraries(photon_sandbox_test PRIVATE photon_core)
 add_executable(photon_scenario_test tests/test_scenario_engine.cpp)
target_link_libraries(photon_scenario_test PRIVATE photon_core)
add_test(NAME photon_scenario COMMAND photon_scenario_test)
add_test(NAME photon_sandbox COMMAND photon_sandbox_test)

#include "photon/ouch5.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>

int main() {
  alignas(64) std::byte dst[64]{};
  const char token[14] = "PHOTON000001";
  const char stock[8] = "TEST    ";
  const char firm[4] = "PHTN";
  constexpr std::uint64_t iters = 5'000'000;
  const auto t0 = std::chrono::steady_clock::now();
  for (std::uint64_t i = 0; i < iters; ++i)
    photon::ouch5::serialize_enter_order(dst, token, 'B', 100, stock, 1234500,
                                          0, firm, 1, 0, 'N');
  const auto t1 = std::chrono::steady_clock::now();
  const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
  std::printf("iterations=%llu total_ns=%lld ns_per_call=%.3f\n",
              (unsigned long long)iters, (long long)ns,
              double(ns) / double(iters));
  return dst[0] == std::byte{'O'} ? 0 : 1;
}

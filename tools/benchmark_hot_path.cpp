#include "photon/ouch5.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>

int main() {
  alignas(64) std::byte dst[64]{};
  const char symbol[8] = {'T','E','S','T',' ',' ',' ',' '};
  const char clord[14] = {'P','H','O','T','O','N','0','0','0','0','0','0','0','1'};
  constexpr std::uint64_t iters = 5'000'000;
  const auto t0 = std::chrono::steady_clock::now();
  for (std::uint64_t i = 0; i < iters; ++i)
    photon::ouch5::serialize_enter_order(dst, std::uint32_t(i+1), 'B', 100,
      symbol, 1234500, '0', 'Y', 'A', 'N', 'N', clord);
  const auto t1 = std::chrono::steady_clock::now();
  const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
  std::printf("iterations=%llu total_ns=%lld ns_per_call=%.3f\n",
              (unsigned long long)iters, (long long)ns,
              double(ns) / double(iters));
  return dst[0] == std::byte{'O'} ? 0 : 1;
}

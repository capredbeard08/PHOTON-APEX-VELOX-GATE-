#pragma once
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

namespace photon::telemetry {

struct alignas(64) Sample {
  std::uint64_t t0{}, t1{}, t2{}, t3{}, t4{};
  std::uint32_t flags{};
  std::uint32_t reserved{};
};
static_assert(sizeof(Sample) == 64);

class SpscRing {
  static constexpr std::uint32_t N = 4096;
  std::array<Sample, N> data_{};
  alignas(64) std::atomic<std::uint32_t> head_{0};
  alignas(64) std::atomic<std::uint32_t> tail_{0};
public:
  bool push(const Sample& s) noexcept {
    const auto h = head_.load(std::memory_order_relaxed);
    const auto n = (h + 1u) & (N - 1u);
    if (n == tail_.load(std::memory_order_acquire)) return false;
    data_[h] = s;
    head_.store(n, std::memory_order_release);
    return true;
  }
  bool pop(Sample& s) noexcept {
    const auto t = tail_.load(std::memory_order_relaxed);
    if (t == head_.load(std::memory_order_acquire)) return false;
    s = data_[t];
    tail_.store((t + 1u) & (N - 1u), std::memory_order_release);
    return true;
  }
};

inline std::uint64_t rdtsc() noexcept {
#if defined(__x86_64__) || defined(_M_X64)
  unsigned lo, hi;
  asm volatile("lfence\nrdtsc" : "=a"(lo), "=d"(hi) :: "memory");
  return (static_cast<std::uint64_t>(hi) << 32) | lo;
#else
  return static_cast<std::uint64_t>(
      std::chrono::steady_clock::now().time_since_epoch().count());
#endif
}

inline std::uint64_t rdtscp() noexcept {
#if defined(__x86_64__) || defined(_M_X64)
  unsigned lo, hi, aux;
  asm volatile("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux) :: "memory");
  asm volatile("lfence" ::: "memory");
  return (static_cast<std::uint64_t>(hi) << 32) | lo;
#else
  return static_cast<std::uint64_t>(
      std::chrono::steady_clock::now().time_since_epoch().count());
#endif
}

// Run once during initialization, never on the hot path.
inline double calibrate_tsc_hz(std::chrono::milliseconds interval =
                                   std::chrono::milliseconds(100)) noexcept {
  const auto wall0 = std::chrono::steady_clock::now();
  const auto c0 = rdtsc();
  std::this_thread::sleep_for(interval);
  const auto c1 = rdtsc();
  const auto wall1 = std::chrono::steady_clock::now();
  const double seconds =
      std::chrono::duration<double>(wall1 - wall0).count();
  return seconds > 0.0 ? static_cast<double>(c1 - c0) / seconds : 0.0;
}

inline std::uint64_t tsc_delta_to_ns(std::uint64_t delta, double tsc_hz) noexcept {
  return tsc_hz > 0.0
      ? static_cast<std::uint64_t>((static_cast<double>(delta) * 1.0e9) / tsc_hz)
      : 0;
}

} // namespace photon::telemetry

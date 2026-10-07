#pragma once
#include <array>
#include <atomic>
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
    auto h = head_.load(std::memory_order_relaxed);
    auto n = (h + 1) & (N - 1);
    if (n == tail_.load(std::memory_order_acquire)) return false;
    data_[h] = s;
    head_.store(n, std::memory_order_release);
    return true;
  }
  bool pop(Sample& s) noexcept {
    auto t = tail_.load(std::memory_order_relaxed);
    if (t == head_.load(std::memory_order_acquire)) return false;
    s = data_[t];
    tail_.store((t + 1) & (N - 1), std::memory_order_release);
    return true;
  }
};

inline std::uint64_t rdtsc() noexcept {
  unsigned lo, hi;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi) :: "memory");
  return (static_cast<std::uint64_t>(hi) << 32) | lo;
}
inline std::uint64_t rdtscp() noexcept {
  unsigned lo, hi, aux;
  asm volatile("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux) :: "memory");
  return (static_cast<std::uint64_t>(hi) << 32) | lo;
}

} // namespace photon::telemetry

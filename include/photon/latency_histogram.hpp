#pragma once
#include <array>
#include <cstdint>

namespace photon::telemetry {

template<std::size_t MaxBuckets = 4096>
class LogHistogram {
  std::array<std::uint64_t, MaxBuckets> buckets_{};
public:
  void observe(std::uint64_t ns) noexcept {
    std::size_t b = 0;
    while (ns > 1 && b + 1 < MaxBuckets) { ns >>= 1; ++b; }
    ++buckets_[b];
  }
  const auto& buckets() const noexcept { return buckets_; }
  void reset() noexcept { buckets_.fill(0); }
};

} // namespace photon::telemetry

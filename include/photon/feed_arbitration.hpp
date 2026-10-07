#pragma once
#include <cstdint>

namespace photon::feed {

// Lock-free single-writer A/B sequence arbiter.
// The caller owns publication to the hot path; no allocation or locks.
class SequenceArbiter {
  std::uint64_t next_{0};
  bool initialized_{false};

public:
  struct Decision {
    bool accept{};
    bool duplicate{};
    bool gap{};
  };

  Decision observe(std::uint64_t seq) noexcept {
    if (!initialized_) {
      initialized_ = true;
      next_ = seq + 1;
      return {true, false, false};
    }
    if (seq < next_) return {false, true, false};
    const bool gap = seq > next_;
    next_ = seq + 1;
    return {true, false, gap};
  }

  std::uint64_t expected() const noexcept { return next_; }
};

} // namespace photon::feed

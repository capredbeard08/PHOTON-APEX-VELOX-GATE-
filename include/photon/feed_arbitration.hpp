#pragma once
#include <cstdint>

namespace photon::feed {

enum class Feed : std::uint8_t { A = 0, B = 1 };

class DualFeedArbiter {
  struct State {
    std::uint64_t last_seq{};
    bool initialized{};
  };
  State a_{}, b_{};
  std::uint64_t accepted_{};
  Feed last_feed_{Feed::A};

  static bool accept_seq(State& s, std::uint64_t seq, bool& duplicate, bool& gap) noexcept {
    duplicate = false;
    gap = false;
    if (!s.initialized) {
      s.initialized = true;
      s.last_seq = seq;
      return true;
    }
    if (seq <= s.last_seq) {
      duplicate = true;
      return false;
    }
    gap = seq != s.last_seq + 1;
    s.last_seq = seq;
    return true;
  }

public:
  struct Decision {
    bool accept{};
    bool duplicate{};
    bool gap{};
    Feed feed{Feed::A};
    std::uint64_t sequence{};
  };

  Decision observe(Feed feed, std::uint64_t sequence) noexcept {
    State& s = (feed == Feed::A) ? a_ : b_;
    bool duplicate = false, gap = false;
    const bool accepted = accept_seq(s, sequence, duplicate, gap);

    // Two multicast copies of the same packet must collapse to one logical event.
    // Once a sequence is accepted on either feed, the other feed's equal/lower
    // sequence is stale and is rejected by its per-feed state plus the global fence.
    if (accepted && sequence > accepted_) {
      accepted_ = sequence;
      last_feed_ = feed;
      return {true, false, gap, feed, sequence};
    }
    if (sequence <= accepted_) duplicate = true;
    return {false, duplicate, gap, feed, sequence};
  }

  std::uint64_t last_accepted() const noexcept { return accepted_; }
  Feed last_source() const noexcept { return last_feed_; }
};

} // namespace photon::feed

#pragma once
#include <array>
#include <cstdint>
#include "photon/market_intelligence.hpp"

namespace photon::scenario {

// Standard research hierarchy from microstructure to one year. Arbitrary
// horizons are still accepted by simulate(); this is the default consensus set.
constexpr std::size_t kTimeframes = 20;
constexpr std::array<std::uint32_t,kTimeframes> kHorizonsMs{
  1000u, 5000u, 10000u, 15000u, 30000u,
  60000u, 300000u, 900000u, 1800000u,
  3600000u, 14400000u, 43200000u,
  86400000u, 259200000u, 604800000u, 1209600000u,
  2592000000u, 7776000000u, 15552000000u, 31536000000u
};

constexpr std::size_t kPathPoints = 16;

struct PathPoint {
  float return_bps{};
  float flow{};
  float volatility{};
  float spread_bps{};
};

struct Scenario {
  float probability{};
  float terminal_return_bps{};
  float max_adverse_bps{};
  float max_favorable_bps{};
  float path_consistency{};
  std::uint32_t horizon_ms{};
  std::uint32_t analogue_count{};
};

struct TimeframeResult {
  std::uint32_t horizon_ms{};
  std::array<Scenario,3> scenarios{}; // down / flat / up
  std::array<PathPoint,kPathPoints> projected_paths{}; // winner path
  std::uint8_t winner{};               // 0 down, 1 flat, 2 up
  float winner_probability{};
  float separation{};
  float path_consistency{};
};

struct Consensus {
  std::array<TimeframeResult,kTimeframes> frames{};
  std::array<PathPoint,kPathPoints> consensus_path{};
  std::uint8_t outcome{}; // 0 down, 1 flat, 2 up
  float consensus_probability{};
  float agreement{};
  float path_agreement{};
  float regime_agreement{};
  bool executable{};
};

struct HistoricalSample {
  intel::MarketSnapshot state{};
  std::array<PathPoint,kPathPoints> path{};
  std::uint8_t path_count{};
  float forward_return_bps{};
  std::uint32_t horizon_ms{};
};

class ScenarioEngine {
public:
  ScenarioEngine() noexcept;
  void reset() noexcept;
  bool ingest(const HistoricalSample&) noexcept;
  TimeframeResult simulate(const intel::MarketSnapshot&, std::uint32_t horizon_ms) const noexcept;
  Consensus cross_validate(const intel::MarketSnapshot&) const noexcept;

private:
  // Live process keeps a bounded analogue cache; the full 1s-1y archive belongs in the offline research store.\n  static constexpr std::size_t kCapacity = 8192;
  std::array<HistoricalSample,kCapacity> samples_{};
  std::uint32_t size_{};
  std::uint32_t cursor_{};
};

} // namespace photon::scenario

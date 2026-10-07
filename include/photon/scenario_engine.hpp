#pragma once
#include <array>
#include <cstdint>
#include "photon/market_intelligence.hpp"

namespace photon::scenario {

constexpr std::size_t kTimeframes = 7;
constexpr std::array<std::uint32_t,kTimeframes> kHorizonsMs{1,5,10,50,100,500,1000};

struct PathPoint { float return_bps{}; float flow{}; float volatility{}; float spread_bps{}; };
struct Scenario {
  float probability{};
  float terminal_return_bps{};
  float max_adverse_bps{};
  float max_favorable_bps{};
  std::uint32_t horizon_ms{};
  std::uint32_t analogue_count{};
};

struct TimeframeResult {
  std::uint32_t horizon_ms{};
  std::array<Scenario,3> scenarios{}; // down / flat / up
  std::uint8_t winner{};               // 0 down, 1 flat, 2 up
  float winner_probability{};
  float separation{};
};

struct Consensus {
  std::array<TimeframeResult,kTimeframes> frames{};
  std::uint8_t outcome{}; // 0 down, 1 flat, 2 up
  float consensus_probability{};
  float agreement{};
  bool executable{};
};

struct HistoricalSample {
  MarketSnapshot state{};
  float forward_return_bps{};
  std::uint32_t horizon_ms{};
};

class ScenarioEngine {
public:
  ScenarioEngine() noexcept;
  void reset() noexcept;
  bool ingest(const HistoricalSample&) noexcept;
  TimeframeResult simulate(const MarketSnapshot&, std::uint32_t horizon_ms) const noexcept;
  Consensus cross_validate(const MarketSnapshot&) const noexcept;

private:
  static constexpr std::size_t kCapacity = 65536;
  std::array<HistoricalSample,kCapacity> samples_{};
  std::uint32_t size_{};
  std::uint32_t cursor_{};
};

} // namespace photon::scenario

#pragma once
#include <cstdint>
#include <string_view>
#include "photon/scenario_engine.hpp"

namespace photon::sandbox {

struct ReplayDecision {
  std::uint64_t timestamp_ns{};
  scenario::Consensus consensus{};
};

class ReplayEngine {
public:
  explicit ReplayEngine(scenario::ScenarioEngine& scenarios) noexcept
      : scenarios_(scenarios) {}

  ReplayDecision evaluate(const intel::MarketSnapshot& snapshot,
                          std::uint64_t timestamp_ns) const noexcept;

  // Deterministic CSV format: timestamp_ns,bid,ask,bid_qty,ask_qty,
  // aggressive_buy,aggressive_sell. Research archives may carry additional
  // columns; parsers should ignore fields they do not consume.
  static bool parse_snapshot(std::string_view line,
                             intel::MarketSnapshot& out) noexcept;

private:
  scenario::ScenarioEngine& scenarios_;
};

} // namespace photon::sandbox

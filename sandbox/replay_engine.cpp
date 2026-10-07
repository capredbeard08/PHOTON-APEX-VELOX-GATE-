#include "replay_engine.hpp"
#include <charconv>
#include <cstdlib>
#include <string>

namespace photon::sandbox {

ReplayDecision ReplayEngine::evaluate(const intel::MarketSnapshot& snapshot,
                                      std::uint64_t timestamp_ns) const noexcept {
  return {timestamp_ns, scenarios_.cross_validate(snapshot)};
}

bool ReplayEngine::parse_snapshot(std::string_view line,
                                  intel::MarketSnapshot& out) noexcept {
  // Minimal deterministic parser for sandbox smoke/replay data. Production
  // ingestion should reconstruct the full L2/L3 book before feature extraction.
  std::size_t p = 0;
  auto next = [&](float& value) noexcept {
    if (p >= line.size()) return false;
    const std::size_t comma = line.find(',', p);
    const std::size_t end = comma == std::string_view::npos ? line.size() : comma;
    const auto token = line.substr(p, end - p);
    char* finish = nullptr;
    std::string tmp(token);
    value = std::strtof(tmp.c_str(), &finish);
    if (finish == tmp.c_str()) return false;
    p = comma == std::string_view::npos ? line.size() : comma + 1;
    return true;
  };

  float timestamp = 0.0f;
  float bid = 0.0f, ask = 0.0f, bq = 0.0f, aq = 0.0f;
  float buy = 0.0f, sell = 0.0f;
  if (!next(timestamp) || !next(bid) || !next(ask) || !next(bq) ||
      !next(aq) || !next(buy) || !next(sell)) return false;

  out = {};
  out.bid_px[0] = bid;
  out.ask_px[0] = ask;
  out.bid_qty[0] = bq;
  out.ask_qty[0] = aq;
  out.aggressive_buy_qty = buy;
  out.aggressive_sell_qty = sell;
  return true;
}

} // namespace photon::sandbox

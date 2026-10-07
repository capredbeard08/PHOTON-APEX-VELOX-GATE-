#include "photon/scenario_engine.hpp"
#include <cassert>
#include <cmath>

int main() {
  static photon::scenario::ScenarioEngine e;
  photon::intel::MarketSnapshot s{};
  s.bid_px.fill(99.99f);
  s.ask_px.fill(100.01f);
  s.bid_qty.fill(100.f);
  s.ask_qty.fill(10.f);
  s.aggressive_buy_qty = 1000.f;
  s.aggressive_sell_qty = 50.f;
  s.added_bid_qty = 500.f;
  s.added_ask_qty = 50.f;
  s.cancelled_ask_qty = 100.f;

  for (auto h : photon::scenario::kHorizonsMs) {
    for (int variant = 0; variant < 4; ++variant) {
      auto x = s;
      x.short_return = 0.0001f * static_cast<float>(variant + 1);
      photon::scenario::HistoricalSample sample{};
      sample.state = x;
      sample.forward_return_bps = variant == 0 ? 2.0f :
                                  variant == 1 ? 1.0f :
                                  variant == 2 ? -0.5f : 0.1f;
      sample.horizon_ms = h;
      sample.path_count = photon::scenario::kPathPoints;
      for (std::size_t p = 0; p < photon::scenario::kPathPoints; ++p) {
        const float sign = variant == 2 ? -1.0f : 1.0f;
        sample.path[p].return_bps = sign * 0.1f * static_cast<float>(p + 1);
        sample.path[p].flow = sign * 0.2f;
        sample.path[p].volatility = 0.5f;
        sample.path[p].spread_bps = 0.02f;
      }
      assert(e.ingest(sample));
    }
  }

  const auto r = e.cross_validate(s);
  assert(r.frames[0].winner_probability >= 0.f);
  assert(r.consensus_probability >= 0.f && r.consensus_probability <= 1.f);
  assert(r.path_agreement >= 0.f && r.path_agreement <= 1.f);
  assert(std::isfinite(r.consensus_path.back().return_bps));
  return 0;
}

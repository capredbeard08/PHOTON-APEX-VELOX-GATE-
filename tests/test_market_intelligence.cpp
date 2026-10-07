#include "photon/market_intelligence.hpp"
#include <cassert>
#include <cmath>
int main() {
  photon::intel::MarketSnapshot s{};
  s.bid_px.fill(99.99f); s.ask_px.fill(100.01f);
  s.bid_qty.fill(100.0f); s.ask_qty.fill(10.0f);
  s.aggressive_buy_qty = 900.0f; s.aggressive_sell_qty = 100.0f;
  s.added_bid_qty = 500.0f; s.added_ask_qty = 100.0f;
  s.cancelled_ask_qty = 200.0f; s.short_return = 0.0002f;
  s.cross_venue_lead_return = 0.0003f; s.related_asset_return = 0.0001f;
  s.short_volatility = 0.0002f; s.spread_bps = 1.0f; s.trade_count = 1000; s.book_updates = 5000;
  const auto f = photon::intel::extract_features(s);
  assert(f.book_imbalance > 0.7f);
  assert(f.flow_imbalance > 0.7f);
  const auto d = photon::intel::infer(f, photon::intel::Model{});
  assert(d.model_version == 1);
  assert(d.p_up > 0.5f);
  assert(std::isfinite(d.executable_edge_bps));
  return 0;
}

#include "photon/market_intelligence.hpp"
#include <algorithm>
#include <cmath>

namespace photon::intel {
namespace {
inline float clamp(float x, float lo, float hi) noexcept {
  return std::max(lo, std::min(x, hi));
}
inline float sigmoid(float x) noexcept {
  x = clamp(x, -18.0f, 18.0f);
  return 1.0f / (1.0f + std::exp(-x));
}
}

Features extract_features(const MarketSnapshot& s) noexcept {
  Features f{};
  float bid=0.0f, ask=0.0f, wbid=0.0f, wask=0.0f;
  for (std::size_t i=0; i<8; ++i) {
    const float w = 1.0f / static_cast<float>(i + 1);
    bid += s.bid_qty[i]; ask += s.ask_qty[i];
    wbid += s.bid_qty[i] * w; wask += s.ask_qty[i] * w;
  }
  const float total = std::max(bid + ask, 1e-6f);
  f.book_imbalance = clamp((bid - ask) / total, -1.0f, 1.0f);

  const float best_bid = s.bid_px[0], best_ask = s.ask_px[0];
  const float mid = (best_bid + best_ask) * 0.5f;
  if (mid > 0.0f && best_ask >= best_bid) {
    const float micro = (best_ask * wbid + best_bid * wask) /
                        std::max(wbid + wask, 1e-6f);
    f.microprice_edge_bps = (micro - mid) / mid * 10000.0f;
  }

  const float trades = std::max(s.aggressive_buy_qty + s.aggressive_sell_qty, 1e-6f);
  f.flow_imbalance = clamp((s.aggressive_buy_qty - s.aggressive_sell_qty) / trades, -1.0f, 1.0f);

  const float cancels = std::max(s.cancelled_bid_qty + s.cancelled_ask_qty, 1e-6f);
  f.cancel_imbalance = clamp((s.cancelled_bid_qty - s.cancelled_ask_qty) / cancels, -1.0f, 1.0f);

  const float adds = std::max(s.added_bid_qty + s.added_ask_qty, 1e-6f);
  f.pressure = clamp(((s.added_bid_qty - s.added_ask_qty) +
                      (s.cancelled_ask_qty - s.cancelled_bid_qty)) /
                     (adds + cancels), -1.0f, 1.0f);

  f.cross_venue_lead = clamp(s.cross_venue_lead_return * 10000.0f, -5.0f, 5.0f);
  f.related_asset_lead = clamp(s.related_asset_return * 10000.0f, -5.0f, 5.0f);
  f.momentum = clamp(s.short_return * 10000.0f, -5.0f, 5.0f);
  f.volatility = clamp(s.short_volatility * 10000.0f, 0.0f, 20.0f);
  f.spread_bps = clamp(s.spread_bps, 0.0f, 50.0f);
  f.activity = clamp(std::log1p(static_cast<float>(s.trade_count + s.book_updates)), 0.0f, 20.0f);
  f.staleness = clamp(s.latency_ms, 0.0f, 1000.0f);
  return f;
}

Decision infer(const Features& f, const Model& m) noexcept {
  const std::array<float,12> x{
    f.book_imbalance, f.microprice_edge_bps, f.flow_imbalance,
    f.cancel_imbalance, f.pressure, f.cross_venue_lead,
    f.related_asset_lead, f.momentum, f.volatility, f.spread_bps,
    f.activity, f.staleness
  };

  float z = m.intercept;
  for (std::size_t i=0; i<x.size(); ++i) z += m.beta[i] * x[i];
  const float p = sigmoid(z);
  const float signed_probability = 2.0f * p - 1.0f;
  const float expected = signed_probability * m.horizon_bps;
  const float costs = m.fee_bps + m.slippage_bps + m.adverse_selection_bps +
                      0.5f * f.spread_bps;
  const float edge = std::fabs(expected) - costs;
  const float confidence = std::fabs(signed_probability);

  Decision d{};
  d.p_up = p;
  d.score = z;
  d.expected_return_bps = expected;
  d.executable_edge_bps = edge;
  d.confidence = confidence;
  d.model_version = m.version;

  if (f.volatility >= 10.0f || f.staleness > 100.0f) d.regime = Regime::Stressed;
  else if (f.volatility <= 1.0f) d.regime = Regime::Calm;
  else d.regime = Regime::Normal;

  if (edge >= m.min_edge_bps && confidence >= m.min_confidence) {
    d.direction = expected > 0.0f ? Direction::Buy : Direction::Sell;
  }
  return d;
}
} // namespace photon::intel

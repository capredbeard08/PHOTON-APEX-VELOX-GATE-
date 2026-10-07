#pragma once
#include <array>
#include <cstdint>
#include <limits>

namespace photon::intel {

enum class Direction : std::uint8_t { Flat, Buy, Sell };
enum class Regime : std::uint8_t { Unknown, Calm, Normal, Stressed };

struct alignas(64) MarketSnapshot {
  std::array<float, 8> bid_qty{};
  std::array<float, 8> ask_qty{};
  std::array<float, 8> bid_px{};
  std::array<float, 8> ask_px{};
  float last_price{};
  float last_qty{};
  float aggressive_buy_qty{};
  float aggressive_sell_qty{};
  float added_bid_qty{};
  float added_ask_qty{};
  float cancelled_bid_qty{};
  float cancelled_ask_qty{};
  float cross_venue_lead_return{};
  float related_asset_return{};
  float short_return{};
  float short_volatility{};
  float spread_bps{};
  float latency_ms{};
  std::uint32_t trade_count{};
  std::uint32_t book_updates{};
};

struct Features {
  float book_imbalance{};
  float microprice_edge_bps{};
  float flow_imbalance{};
  float cancel_imbalance{};
  float pressure{};
  float cross_venue_lead{};
  float related_asset_lead{};
  float momentum{};
  float volatility{};
  float spread_bps{};
  float activity{};
  float staleness{};
};

struct Decision {
  Direction direction{Direction::Flat};
  Regime regime{Regime::Unknown};
  float p_up{0.5f};
  float expected_return_bps{};
  float executable_edge_bps{};
  float confidence{};
  float score{};
  std::uint32_t model_version{};
};

struct Model {
  // Coefficients are versioned and must be fitted offline with purged,
  // walk-forward validation. These are deliberately conservative defaults.
  std::array<float, 12> beta{
      0.85f, 0.35f, 0.75f, 0.20f, 0.45f, 0.55f,
      0.30f, 0.40f, -0.65f, -0.55f, 0.12f, -0.80f
  };
  float intercept{};
  float horizon_bps{1.0f};
  float fee_bps{0.10f};
  float slippage_bps{0.20f};
  float adverse_selection_bps{0.25f};
  float min_edge_bps{0.35f};
  float min_confidence{0.55f};
  std::uint32_t version{1};
};

Features extract_features(const MarketSnapshot&) noexcept;
Decision infer(const Features&, const Model&) noexcept;

} // namespace photon::intel

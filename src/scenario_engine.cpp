#include "photon/scenario_engine.hpp"
#include <algorithm>
#include <cmath>

namespace photon::scenario {
namespace {

struct Candidate {
  float distance{};
  float return_bps{};
  std::uint32_t index{};
};

float state_distance(const intel::MarketSnapshot& a,
                     const intel::MarketSnapshot& b) noexcept {
  const auto fa = intel::extract_features(a);
  const auto fb = intel::extract_features(b);
  float d = 0.0f;
  d += std::abs(fa.book_imbalance - fb.book_imbalance) * 2.0f;
  d += std::abs(fa.flow_imbalance - fb.flow_imbalance) * 2.0f;
  d += std::abs(fa.cancel_imbalance - fb.cancel_imbalance);
  d += std::abs(fa.pressure - fb.pressure);
  d += std::abs(fa.cross_venue_lead - fb.cross_venue_lead) * 0.1f;
  d += std::abs(fa.momentum - fb.momentum) * 0.1f;
  d += std::abs(fa.volatility - fb.volatility) * 0.05f;
  d += std::abs(fa.spread_bps - fb.spread_bps) * 0.05f;
  return d;
}

float path_distance(const std::array<PathPoint,kPathPoints>& a,
                    const std::array<PathPoint,kPathPoints>& b,
                    std::size_t n) noexcept {
  if (n == 0) return 0.0f;
  float d = 0.0f;
  for (std::size_t i = 0; i < n; ++i) {
    d += std::abs(a[i].return_bps - b[i].return_bps) * 0.05f;
    d += std::abs(a[i].flow - b[i].flow) * 0.20f;
    d += std::abs(a[i].volatility - b[i].volatility) * 0.10f;
    d += std::abs(a[i].spread_bps - b[i].spread_bps) * 0.05f;
  }
  return d / static_cast<float>(n);
}

int bucket(float r) noexcept {
  return r > 0.25f ? 2 : (r < -0.25f ? 0 : 1);
}

} // namespace

ScenarioEngine::ScenarioEngine() noexcept = default;
void ScenarioEngine::reset() noexcept { size_ = cursor_ = 0; }

bool ScenarioEngine::ingest(const HistoricalSample& sample) noexcept {
  if (sample.horizon_ms == 0) return false;
  samples_[cursor_] = sample;
  cursor_ = (cursor_ + 1U) % static_cast<std::uint32_t>(kCapacity);
  if (size_ < kCapacity) ++size_;
  return true;
}

TimeframeResult ScenarioEngine::simulate(const intel::MarketSnapshot& now,
                                         std::uint32_t horizon) const noexcept {
  TimeframeResult out{};
  out.horizon_ms = horizon;
  if (horizon == 0 || size_ == 0) return out;

  // Keep a bounded nearest-neighbour set without sorting the entire database.
  constexpr std::size_t kNearest = 64;
  std::array<Candidate,kNearest> nearest{};
  std::size_t count = 0;

  for (std::uint32_t i = 0; i < size_; ++i) {
    const auto& sample = samples_[i];
    if (sample.horizon_ms != horizon) continue;

    const float d = state_distance(now, sample.state);
    if (count < kNearest) {
      nearest[count++] = {d, sample.forward_return_bps, i};
      continue;
    }

    std::size_t worst = 0;
    for (std::size_t j = 1; j < kNearest; ++j)
      if (nearest[j].distance > nearest[worst].distance) worst = j;
    if (d < nearest[worst].distance)
      nearest[worst] = {d, sample.forward_return_bps, i};
  }

  if (count == 0) return out;

  float total_weight = 0.0f;
  float weights[3]{};
  float returns[3]{};
  float path_consistency[3]{};
  std::array<float,kPathPoints> path_weight{};
  std::array<PathPoint,kPathPoints> path_sum{};

  for (std::size_t i = 0; i < count; ++i) {
    const auto& c = nearest[i];
    const auto& sample = samples_[c.index];
    const float w = 1.0f / (0.001f + c.distance);
    const int b = bucket(c.return_bps);

    total_weight += w;
    weights[b] += w;
    returns[b] += w * c.return_bps;

    // Re-simulate the analogue path, not merely its endpoint. The path is
    // weighted by similarity so transient adverse/favourable excursions survive.
    if (sample.path_count > 0) {
      const std::size_t n = std::min<std::size_t>(sample.path_count, kPathPoints);
      for (std::size_t p = 0; p < n; ++p) {
        path_sum[p].return_bps += w * sample.path[p].return_bps;
        path_sum[p].flow += w * sample.path[p].flow;
        path_sum[p].volatility += w * sample.path[p].volatility;
        path_sum[p].spread_bps += w * sample.path[p].spread_bps;
        path_weight[p] += w;
      }
    }

    // A second similarity pass measures whether the analogue's path agrees
    // with the local directional shape. This penalizes endpoint-only matches.
    const auto current_features = intel::extract_features(now);
    std::array<PathPoint,kPathPoints> anchor{};
    anchor[0].flow = current_features.flow_imbalance;
    anchor[0].volatility = current_features.volatility;
    anchor[0].spread_bps = current_features.spread_bps;
    const float pd = path_distance(anchor, sample.path, std::min<std::size_t>(sample.path_count, 1));
    path_consistency[b] += w / (1.0f + pd);
  }

  for (int b = 0; b < 3; ++b) {
    auto& scenario = out.scenarios[b];
    scenario.horizon_ms = horizon;
    scenario.probability = weights[b] / std::max(total_weight, 1e-6f);
    scenario.terminal_return_bps = returns[b] / std::max(weights[b], 1e-6f);
    scenario.analogue_count = static_cast<std::uint32_t>(count);
    scenario.path_consistency = path_consistency[b] / std::max(weights[b], 1e-6f);
  }

  int winner = 0;
  for (int b = 1; b < 3; ++b)
    if (out.scenarios[b].probability > out.scenarios[winner].probability) winner = b;

  out.winner = static_cast<std::uint8_t>(winner);
  out.winner_probability = out.scenarios[winner].probability;
  const float second = std::max(
      out.scenarios[(winner + 1) % 3].probability,
      out.scenarios[(winner + 2) % 3].probability);
  out.separation = out.winner_probability - second;

  // Re-simulate the winning path as a weighted continuation.
  const auto& winning = out.scenarios[winner];
  float winner_weight = 0.0f;
  for (std::size_t i = 0; i < count; ++i) {
    const auto& c = nearest[i];
    if (bucket(c.return_bps) != winner) continue;
    const float w = 1.0f / (0.001f + c.distance);
    winner_weight += w;
    const auto& sample = samples_[c.index];
    const std::size_t n = std::min<std::size_t>(sample.path_count, kPathPoints);
    for (std::size_t p = 0; p < n; ++p) {
      out.projected_paths[p].return_bps += w * sample.path[p].return_bps;
      out.projected_paths[p].flow += w * sample.path[p].flow;
      out.projected_paths[p].volatility += w * sample.path[p].volatility;
      out.projected_paths[p].spread_bps += w * sample.path[p].spread_bps;
    }
  }
  for (std::size_t p = 0; p < kPathPoints; ++p) {
    if (winner_weight > 0.0f) {
      out.projected_paths[p].return_bps /= winner_weight;
      out.projected_paths[p].flow /= winner_weight;
      out.projected_paths[p].volatility /= winner_weight;
      out.projected_paths[p].spread_bps /= winner_weight;
    }
  }

  // Populate excursion statistics from the simulated winning path.
  float favourable = 0.0f;
  float adverse = 0.0f;
  for (const auto& point : out.projected_paths) {
    favourable = std::max(favourable, point.return_bps);
    adverse = std::min(adverse, point.return_bps);
  }
  out.scenarios[winner].max_favorable_bps = favourable;
  out.scenarios[winner].max_adverse_bps = adverse;
  out.path_consistency = out.scenarios[winner].path_consistency;
  return out;
}

Consensus ScenarioEngine::cross_validate(const intel::MarketSnapshot& now) const noexcept {
  Consensus c{};
  float votes[3]{};
  float probsum = 0.0f;
  float pathsum[3]{};
  float regime_votes[3]{};

  for (std::size_t i = 0; i < kTimeframes; ++i) {
    c.frames[i] = simulate(now, kHorizonsMs[i]);
    const auto& frame = c.frames[i];
    if (frame.winner_probability <= 0.0f) continue;

    const float quality = std::max(0.0f, frame.separation) *
                          std::max(0.0f, frame.path_consistency);
    const float vote = frame.winner_probability * (0.25f + quality);
    votes[frame.winner] += vote;
    pathsum[frame.winner] += frame.path_consistency;
    probsum += vote;

    // Short horizons are execution-sensitive; longer horizons are regime
    // confirmation. Both are required for executable consensus.
    const float regime_weight = i < 5 ? 0.5f : 1.0f;
    regime_votes[frame.winner] += regime_weight;
  }

  int winner = 0;
  for (int b = 1; b < 3; ++b)
    if (votes[b] > votes[winner]) winner = b;

  c.outcome = static_cast<std::uint8_t>(winner);
  c.consensus_probability = votes[winner] / std::max(probsum, 1e-6f);

  float total_frames = 0.0f;
  for (std::size_t i = 0; i < kTimeframes; ++i)
    if (c.frames[i].winner_probability > 0.0f) total_frames += 1.0f;

  c.agreement = votes[winner] / std::max(probsum, 1e-6f);
  c.path_agreement = pathsum[winner] /
                     std::max(1.0f, total_frames);
  c.regime_agreement = regime_votes[winner] /
                       std::max(1.0f, 1.5f * total_frames);

  // Do not collapse disagreement into a forced prediction. The winner must
  // survive endpoint, path-shape, and regime checks across the hierarchy.
  c.executable =
      total_frames >= 3.0f &&
      c.consensus_probability >= 0.62f &&
      c.agreement >= 0.55f &&
      c.path_agreement >= 0.20f &&
      c.regime_agreement >= 0.35f;

  // Cross-timeframe re-simulation: compare the winning path against the
  // projected paths from every populated timeframe. A stable direction at
  // both the micro and macro ends is stronger than a majority of adjacent
  // horizons.
  if (c.executable) {
    const auto& reference = c.frames[0].projected_paths;
    float agreement_count = 0.0f;
    float compared = 0.0f;
    for (std::size_t i = 0; i < kTimeframes; ++i) {
      if (c.frames[i].winner_probability <= 0.0f) continue;
      const float a = reference.back().return_bps;
      const float b = c.frames[i].projected_paths.back().return_bps;
      const bool same = winner == 2 ? (a > 0.0f && b > 0.0f)
                                    : winner == 0 ? (a < 0.0f && b < 0.0f)
                                                  : (std::abs(a) < 0.25f && std::abs(b) < 0.25f);
      agreement_count += same ? 1.0f : 0.0f;
      compared += 1.0f;
    }
    c.path_agreement = agreement_count / std::max(1.0f, compared);
    c.executable = c.path_agreement >= 0.55f;
  }

  for (std::size_t p = 0; p < kPathPoints; ++p) {
    float wsum = 0.0f;
    for (std::size_t i = 0; i < kTimeframes; ++i) {
      if (c.frames[i].winner != winner || c.frames[i].winner_probability <= 0.0f) continue;
      const float w = c.frames[i].winner_probability;
      c.consensus_path[p].return_bps += w * c.frames[i].projected_paths[p].return_bps;
      c.consensus_path[p].flow += w * c.frames[i].projected_paths[p].flow;
      c.consensus_path[p].volatility += w * c.frames[i].projected_paths[p].volatility;
      c.consensus_path[p].spread_bps += w * c.frames[i].projected_paths[p].spread_bps;
      wsum += w;
    }
    if (wsum > 0.0f) {
      c.consensus_path[p].return_bps /= wsum;
      c.consensus_path[p].flow /= wsum;
      c.consensus_path[p].volatility /= wsum;
      c.consensus_path[p].spread_bps /= wsum;
    }
  }

  return c;
}

} // namespace photon::scenario

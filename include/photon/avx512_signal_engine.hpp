#pragma once
#include <array>
#include <cstdint>

namespace photon::signal {
struct Book8 {
  alignas(64) std::array<float, 8> bid{};
  alignas(64) std::array<float, 8> ask{};
  alignas(64) std::array<float, 8> bid_qty{};
  alignas(64) std::array<float, 8> ask_qty{};
};

float imbalance(const Book8&) noexcept;
float max_imbalance(const Book8&) noexcept;
float weighted_imbalance(const Book8&) noexcept;
} // namespace photon::signal

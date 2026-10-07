#include "photon/ouch5.hpp"
#include "photon/nanosecond_telemetry.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace photon::gateway {
struct alignas(64) DmaSlot { std::array<std::byte, 64> bytes{}; };

class Gateway {
  std::array<DmaSlot, 1024> ring_{};
  std::uint32_t cursor_{};
public:
  std::byte* next_slot() noexcept {
    auto& s = ring_[cursor_++ & 1023u];
    return s.bytes.data();
  }
  void build_enter_order(std::byte* dst, const char token[14], char side,
                         std::uint32_t qty, const char stock[8],
                         std::uint32_t price, std::uint32_t tif,
                         const char firm[4], std::uint32_t display,
                         std::uint16_t capacity, char intermarket) noexcept {
    photon::ouch5::serialize_enter_order(dst, token, side, qty, stock, price,
                                          tif, firm, display, capacity, intermarket);
  }
};
} // namespace photon::gateway

#include "photon/ouch5.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace photon::gateway {

struct alignas(64) DmaSlot { std::array<std::byte, 64> bytes{}; };
static_assert(alignof(DmaSlot) == 64);

class Gateway {
  std::array<DmaSlot, 1024> ring_{};
  std::uint32_t cursor_{};

public:
  std::byte* next_slot() noexcept {
    auto& s = ring_[cursor_++ & 1023u];
    return s.bytes.data();
  }

  void build_enter_order(
      std::byte* dst, std::uint32_t user_ref_num, char side,
      std::uint32_t qty, const char symbol[8], std::uint64_t price,
      char tif, char display, char capacity, char iso, char cross,
      const char clord_id[14], const std::byte* appendage = nullptr,
      std::uint16_t appendage_length = 0) noexcept {
    photon::ouch5::serialize_enter_order(
        dst, user_ref_num, side, qty, symbol, price, tif, display,
        capacity, iso, cross, clord_id, appendage, appendage_length);
  }
};

} // namespace photon::gateway

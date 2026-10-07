#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace photon::ouch5 {

// Nasdaq OUCH 5.0 (October 2025) Enter Order fixed portion.
// Offsets: Type 0, UserRefNum 1, Side 5, Quantity 6, Symbol 10,
// Price 18 (8-byte Price), TIF 26, Display 27, Capacity 28,
// ISO 29, CrossType 30, ClOrdID 31, AppendageLength 45.
// Optional appendages begin at offset 47.
#pragma pack(push, 1)
struct EnterOrder {
  char type;
  std::uint32_t user_ref_num_be;
  char side;
  std::uint32_t quantity_be;
  char symbol[8];
  std::uint64_t price_be;
  char time_in_force;
  char display;
  char capacity;
  char intermarket_sweep;
  char cross_type;
  char clord_id[14];
  std::uint16_t appendage_length_be;
};
#pragma pack(pop)
static_assert(sizeof(EnterOrder) == 47);
static_assert(offsetof(EnterOrder, user_ref_num_be) == 1);
static_assert(offsetof(EnterOrder, price_be) == 18);
static_assert(offsetof(EnterOrder, appendage_length_be) == 45);

constexpr std::uint32_t bswap32(std::uint32_t v) noexcept { return __builtin_bswap32(v); }
constexpr std::uint64_t bswap64(std::uint64_t v) noexcept { return __builtin_bswap64(v); }
constexpr std::uint16_t bswap16(std::uint16_t v) noexcept { return __builtin_bswap16(v); }

inline void serialize_enter_order(
    std::byte* dst, std::uint32_t user_ref_num, char side,
    std::uint32_t quantity, const char symbol[8], std::uint64_t price,
    char time_in_force, char display, char capacity,
    char intermarket_sweep, char cross_type, const char clord_id[14],
    const std::byte* appendage = nullptr, std::uint16_t appendage_length = 0) noexcept {

  EnterOrder w{};
  w.type = 'O';
  w.user_ref_num_be = bswap32(user_ref_num);
  w.side = side;
  w.quantity_be = bswap32(quantity);
  std::memcpy(w.symbol, symbol, sizeof w.symbol);
  w.price_be = bswap64(price);
  w.time_in_force = time_in_force;
  w.display = display;
  w.capacity = capacity;
  w.intermarket_sweep = intermarket_sweep;
  w.cross_type = cross_type;
  std::memcpy(w.clord_id, clord_id, sizeof w.clord_id);
  w.appendage_length_be = bswap16(appendage_length);
  std::memcpy(dst, &w, sizeof w);
  if (appendage_length != 0 && appendage != nullptr)
    std::memcpy(dst + sizeof w, appendage, appendage_length);
}

} // namespace photon::ouch5

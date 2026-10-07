#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace photon::ouch5 {

// OUCH 5.0 Enter Order fixed portion is 47 bytes; appendages follow it.
// All multi-byte wire fields are network byte order.
#pragma pack(push, 1)
struct EnterOrder {
  char     type;                 // 0
  char     token[14];            // 1
  char     side;                 // 15
  std::uint32_t shares_be;       // 16
  char     stock[8];             // 20
  std::uint32_t price_be;        // 28 (price in 1/10000)
  std::uint32_t time_in_force_be;// 32
  char     firm[4];              // 36
  std::uint32_t display_be;      // 40
  std::uint16_t capacity_be;     // 44
  char     intermarket;          // 46
};
#pragma pack(pop)
static_assert(sizeof(EnterOrder) == 47);

constexpr std::uint32_t bswap32(std::uint32_t v) noexcept {
  return __builtin_bswap32(v);
}
constexpr std::uint16_t bswap16(std::uint16_t v) noexcept {
  return __builtin_bswap16(v);
}

inline void serialize_enter_order(
    std::byte* dst, const char token[14], char side,
    std::uint32_t shares, const char stock[8], std::uint32_t price,
    std::uint32_t tif, const char firm[4], std::uint32_t display,
    std::uint16_t capacity, char intermarket) noexcept {
  EnterOrder w{};
  w.type = 'O';
  std::memcpy(w.token, token, 14);
  w.side = side;
  w.shares_be = bswap32(shares);
  std::memcpy(w.stock, stock, 8);
  w.price_be = bswap32(price);
  w.time_in_force_be = bswap32(tif);
  std::memcpy(w.firm, firm, 4);
  w.display_be = bswap32(display);
  w.capacity_be = bswap16(capacity);
  w.intermarket = intermarket;
  std::memcpy(dst, &w, sizeof(w));
}

} // namespace photon::ouch5

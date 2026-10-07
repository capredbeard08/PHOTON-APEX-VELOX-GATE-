#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <string_view>

namespace photon::broker {

enum class Protocol : std::uint8_t { ouch, fix42, fix44, rest, websocket };
struct Route {
  std::uint16_t strategy{};
  std::uint16_t account{};
  std::uint16_t clearing_firm{};
  std::uint16_t mpid{};
  std::uint16_t subaccount{};
};

struct alignas(64) SessionState {
  std::atomic<std::uint32_t> heartbeat{0};
  std::atomic<std::uint32_t> connected{0};
  std::atomic<std::uint64_t> last_rx_tsc{0};
  std::atomic<std::uint64_t> last_tx_tsc{0};
};

class SessionManager {
  SessionState state_{};
  std::array<Route, 256> routes_{};
public:
  bool connected() const noexcept { return state_.connected.load(std::memory_order_acquire) != 0; }
  void set_connected(bool v) noexcept {
    state_.connected.store(v ? 1u : 0u, std::memory_order_release);
  }
  void heartbeat_rx(std::uint64_t tsc) noexcept {
    state_.last_rx_tsc.store(tsc, std::memory_order_relaxed);
    state_.heartbeat.fetch_add(1, std::memory_order_relaxed);
  }
  void heartbeat_tx(std::uint64_t tsc) noexcept {
    state_.last_tx_tsc.store(tsc, std::memory_order_relaxed);
  }
  void set_route(std::uint16_t id, Route r) noexcept { routes_[id & 255u] = r; }
  const Route& route(std::uint16_t id) const noexcept { return routes_[id & 255u]; }
};

} // namespace photon::broker

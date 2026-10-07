#include "photon/ouch5.hpp"
#include "photon/avx512_signal_engine.hpp"
#include "photon/feed_arbitration.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>

int main() {
  alignas(64) std::byte buf[64]{};
  const char symbol[8] = {'T','E','S','T',' ',' ',' ',' '};
  const char clord[14] = {'P','H','O','T','O','N','0','0','0','0','0','0','0','1'};

  photon::ouch5::serialize_enter_order(
      buf, 1, 'B', 100, symbol, 1234500, '0', 'Y', 'A', 'N', 'N', clord);

  assert(buf[0] == std::byte{'O'});
  assert(buf[1] == std::byte{0});
  assert(buf[4] == std::byte{1});
  assert(buf[5] == std::byte{'B'});
  assert(buf[9] == std::byte{100});
  assert(buf[18] == std::byte{0});
  assert(buf[25] == std::byte{68}); // low byte of 1,234,500 in big-endian form
  assert(buf[26] == std::byte{'0'});
  assert(buf[27] == std::byte{'Y'});
  assert(buf[28] == std::byte{'A'});
  assert(buf[29] == std::byte{'N'});
  assert(buf[30] == std::byte{'N'});
  assert(buf[45] == std::byte{0});
  assert(buf[46] == std::byte{0});

  photon::signal::Book8 b{};
  for (int i = 0; i < 8; ++i) { b.bid_qty[i] = 10.0f; b.ask_qty[i] = 5.0f; }
  assert(photon::signal::imbalance(b) > 0.66f);
  assert(photon::signal::max_imbalance(b) > 0.66f);

  photon::feed::DualFeedArbiter arb;
  auto d = arb.observe(photon::feed::Feed::A, 100);
  assert(d.accept && !d.duplicate);
  d = arb.observe(photon::feed::Feed::B, 100);
  assert(!d.accept && d.duplicate);
  d = arb.observe(photon::feed::B, 101);
  assert(d.accept && !d.duplicate);
  d = arb.observe(photon::feed::A, 101);
  assert(!d.accept && d.duplicate);
  return 0;
}

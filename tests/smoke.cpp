#include "photon/ouch5.hpp"
#include "photon/avx512_signal_engine.hpp"
#include "photon/feed_arbitration.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

int main() {
  alignas(64) std::byte buf[64]{};
  const char symbol[8] = {'T','E','S','T',' ',' ',' ',' '};
  const char clord[14] = {'P','H','O','T','O','N','0','0','0','0','0','0','0','1'};
  photon::ouch5::serialize_enter_order(buf, 1, 'B', 100, symbol, 1234500,
                                        '0', 'Y', 'A', 'N', 'N', clord);
  assert(buf[0] == std::byte{'O'});
  assert(buf[1] == std::byte{0});
  assert(buf[4] == std::byte{1});
  assert(buf[18] == std::byte{0});
  assert(buf[25] == std::byte{0});
  assert(buf[45] == std::byte{0});
  assert(buf[46] == std::byte{0});

  photon::signal::Book8 b{};
  for (int i=0;i<8;++i) { b.bid_qty[i]=10.0f; b.ask_qty[i]=5.0f; }
  assert(photon::signal::imbalance(b) > 0.66f);
  assert(photon::signal::max_imbalance(b) > 0.66f);

  photon::feed::SequenceArbiter arb;
  auto a=arb.observe(100); assert(a.accept && !a.duplicate);
  a=arb.observe(100); assert(!a.accept && a.duplicate);
  a=arb.observe(102); assert(a.accept && a.gap);
  return 0;
}

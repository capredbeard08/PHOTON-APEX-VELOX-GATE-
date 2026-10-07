#include "photon/ouch5.hpp"
#include "photon/avx512_signal_engine.hpp"
#include <cassert>
#include <cstddef>
#include <cstring>
int main() {
  std::byte buf[64]{};
  const char token[14] = "PHOTON000001";
  const char stock[8] = "TEST    ";
  const char firm[4] = "PHTN";
  photon::ouch5::serialize_enter_order(buf, token, 'B', 100, stock, 1234500,
                                        0, firm, 1, 0, 'N');
  assert(buf[0] == std::byte{'O'});
  return 0;
}

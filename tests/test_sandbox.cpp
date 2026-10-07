#include "replay_engine.hpp"
#include <cassert>

int main() {
  photon::scenario::ScenarioEngine scenarios;
  photon::intel::MarketSnapshot s{};
  s.bid_px[0] = 99.99f;
  s.ask_px[0] = 100.01f;
  s.bid_qty[0] = 100.0f;
  s.ask_qty[0] = 10.0f;
  photon::scenario::HistoricalSample h{};
  h.state = s;
  h.forward_return_bps = 1.0f;
  h.horizon_ms = 1000;
  h.path_count = 1;
  h.path[0].return_bps = 1.0f;
  assert(scenarios.ingest(h));

  photon::sandbox::ReplayEngine replay(scenarios);
  const auto d = replay.evaluate(s, 123456789ULL);
  assert(d.timestamp_ns == 123456789ULL);

  photon::intel::MarketSnapshot parsed{};
  assert(photon::sandbox::ReplayEngine::parse_snapshot(
      "123,99.99,100.01,100,10,20,3", parsed));
  assert(parsed.bid_px[0] == 99.99f);
  return 0;
}

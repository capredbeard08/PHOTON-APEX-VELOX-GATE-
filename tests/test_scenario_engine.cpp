#include "photon/scenario_engine.hpp"
#include <cassert>
int main() {
  photon::scenario::ScenarioEngine e;
  photon::intel::MarketSnapshot s{};
  s.bid_px.fill(99.99f); s.ask_px.fill(100.01f);
  s.bid_qty.fill(100.f); s.ask_qty.fill(10.f);
  s.aggressive_buy_qty=1000.f; s.aggressive_sell_qty=50.f;
  s.added_bid_qty=500.f; s.added_ask_qty=50.f;
  s.cancelled_ask_qty=100.f;
  for(auto h: photon::scenario::kHorizonsMs) {
    auto x=s; x.short_return=0.0001f*static_cast<float>(h);
    e.ingest({x,2.0f, h});
    e.ingest({x,1.0f, h});
    e.ingest({x,-0.5f, h});
  }
  const auto r=e.cross_validate(s);
  assert(r.frames[0].winner_probability>=0.f);
  assert(r.consensus_probability>=0.f && r.consensus_probability<=1.f);
  return 0;
}

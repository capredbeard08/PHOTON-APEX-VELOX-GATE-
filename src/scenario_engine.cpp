#include "photon/scenario_engine.hpp"
#include <algorithm>
#include <cmath>

namespace photon::scenario {
namespace {
float distance(const intel::MarketSnapshot& a,const intel::MarketSnapshot& b) noexcept {
  const auto fa=intel::extract_features(a), fb=intel::extract_features(b);
  float d=0;
  d+=std::abs(fa.book_imbalance-fb.book_imbalance)*2.0f;
  d+=std::abs(fa.flow_imbalance-fb.flow_imbalance)*2.0f;
  d+=std::abs(fa.cancel_imbalance-fb.cancel_imbalance);
  d+=std::abs(fa.pressure-fb.pressure);
  d+=std::abs(fa.cross_venue_lead-fb.cross_venue_lead)*0.1f;
  d+=std::abs(fa.momentum-fb.momentum)*0.1f;
  d+=std::abs(fa.volatility-fb.volatility)*0.05f;
  d+=std::abs(fa.spread_bps-fb.spread_bps)*0.05f;
  return d;
}
float sigmoid(float x) noexcept { x=std::max(-12.f,std::min(12.f,x)); return 1.f/(1.f+std::exp(-x)); }
}

ScenarioEngine::ScenarioEngine() noexcept = default;
void ScenarioEngine::reset() noexcept { size_=cursor_=0; }

bool ScenarioEngine::ingest(const HistoricalSample& s) noexcept {
  samples_[cursor_]=s;
  cursor_=(cursor_+1)%kCapacity;
  if(size_<kCapacity) ++size_;
  return true;
}

TimeframeResult ScenarioEngine::simulate(const intel::MarketSnapshot& now,std::uint32_t horizon) const noexcept {
  struct Candidate { float d; float r; };
  std::array<Candidate,kCapacity> nearest{};
  std::size_t n=0;
  for(std::uint32_t i=0;i<size_;++i) {
    if(samples_[i].horizon_ms!=horizon) continue;
    const float d=distance(now,samples_[i].state);
    if(n<nearest.size()) nearest[n++]={d,samples_[i].forward_return_bps};
  }
  std::sort(nearest.begin(),nearest.begin()+n,[](auto&a,auto&b){return a.d<b.d;});
  const std::size_t k=std::min<std::size_t>(n,64);
  TimeframeResult out{}; out.horizon_ms=horizon;
  if(!k) return out;

  float wsum=0, means[3]{}, weights[3]{};
  float favourable[3]{}, adverse[3]{};
  for(std::size_t i=0;i<k;++i) {
    const float w=1.f/(0.001f+nearest[i].d);
    const float r=nearest[i].r;
    const int bucket=r>0.25f?2:(r<-0.25f?0:1);
    weights[bucket]+=w; means[bucket]+=w*r; wsum+=w;
  }
  for(int b=0;b<3;++b) {
    out.scenarios[b].horizon_ms=horizon;
    out.scenarios[b].probability=weights[b]/std::max(wsum,1e-6f);
    out.scenarios[b].terminal_return_bps=means[b]/std::max(weights[b],1e-6f);
    out.scenarios[b].analogue_count=static_cast<std::uint32_t>(k);
  }
  int winner=0;
  for(int b=1;b<3;++b) if(out.scenarios[b].probability>out.scenarios[winner].probability) winner=b;
  out.winner=static_cast<std::uint8_t>(winner);
  out.winner_probability=out.scenarios[winner].probability;
  const float second=std::max(out.scenarios[(winner+1)%3].probability,
                              out.scenarios[(winner+2)%3].probability);
  out.separation=out.winner_probability-second;
  return out;
}

Consensus ScenarioEngine::cross_validate(const intel::MarketSnapshot& now) const noexcept {
  Consensus c{};
  float votes[3]{}, probsum=0;
  for(std::size_t i=0;i<kTimeframes;++i) {
    c.frames[i]=simulate(now,kHorizonsMs[i]);
    const auto& f=c.frames[i];
    votes[f.winner]+=std::max(0.f,f.winner_probability);
    probsum+=f.winner_probability;
  }
  int winner=0;
  for(int b=1;b<3;++b) if(votes[b]>votes[winner]) winner=b;
  c.outcome=static_cast<std::uint8_t>(winner);
  c.consensus_probability=votes[winner]/std::max(probsum,1e-6f);
  c.agreement=votes[winner]/std::max(1.f,static_cast<float>(kTimeframes));
  c.executable=(c.consensus_probability>=0.62f && c.agreement>=0.45f);
  return c;
}
} // namespace photon::scenario

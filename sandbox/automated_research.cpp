#include "automated_research.hpp"
#include "replay_engine.hpp"
#include <fstream>
#include <iomanip>
namespace photon::sandbox {
ResearchRunResult AutomatedResearch::run(const ResearchRunConfig& cfg) noexcept {
  ResearchRunResult r{}; std::ifstream in(cfg.input_path); if (!in) return r; std::ofstream out(cfg.output_path); if (!out) return r;
  out<<"timestamp_ns,decision,probability,agreement,path_agreement,regime_agreement\n"; std::string line;
  while(std::getline(in,line)){ ++r.rows_read; if(line.empty()||line[0]=='#') continue; intel::MarketSnapshot s{}; if(!ReplayEngine::parse_snapshot(line,s)){++r.parse_errors;continue;} auto d=engine_.cross_validate(s); ++r.rows_replayed; if(d.executable)++r.executable_decisions;else++r.no_trade_decisions; out<<"0,"<<static_cast<int>(d.outcome)<<","<<std::setprecision(9)<<d.consensus_probability<<","<<d.agreement<<","<<d.path_agreement<<","<<d.regime_agreement<<"\n"; }
  if(r.rows_replayed) r.executable_rate=static_cast<double>(r.executable_decisions)/r.rows_replayed; return r;
}
}

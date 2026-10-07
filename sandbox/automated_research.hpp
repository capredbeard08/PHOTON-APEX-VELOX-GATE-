#pragma once
#include <cstdint>
#include <string>
#include "photon/scenario_engine.hpp"
namespace photon::sandbox {
struct ResearchRunConfig { std::string input_path, output_path; std::uint64_t start_epoch_ns{}, end_epoch_ns{}, embargo_ns{}; bool walk_forward{true}, purge{true}; };
struct ResearchRunResult { std::uint64_t rows_read{}, rows_replayed{}, executable_decisions{}, no_trade_decisions{}, parse_errors{}; double executable_rate{}; };
class AutomatedResearch { public: explicit AutomatedResearch(scenario::ScenarioEngine& e) noexcept : engine_(e) {} ResearchRunResult run(const ResearchRunConfig&) noexcept; private: scenario::ScenarioEngine& engine_; };
}

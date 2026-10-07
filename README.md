# PHOTON-APEX-VELOX-GATE-

Hardware-accelerated tick-to-trade HFT engineering baseline combining C++20, AVX-512-capable signal processing, FPGA risk/transport logic, kernel-bypass networking and nanosecond telemetry.

## Status

The repository now contains the first source baseline. Performance figures are **targets**, not claims: sub-100 ns wire-to-wire execution must be demonstrated with the deployed CPU/NIC/FPGA, clocking, network and venue path.

## Layout

- `include/photon/` — hot-path interfaces
- `src/` — C++ gateway and signal engine
- `hw/` — SystemVerilog risk/transport blocks
- `tools/` — latency analysis
- `tests/` — software and FPGA tests
- `docs/` — PRD and SLA
- `scripts/` — host tuning examples
- `config/` — reviewed host-configuration examples

## Build

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Safety and deployment

Never commit API keys, broker credentials, certificates or private signing material. Live order transmission remains disabled until protocol conformance, risk-gate simulation, packet-capture validation, timestamp calibration and venue certification are complete.

## Roadmap

1. Protocol-accurate OUCH appendages and conformance vectors.
2. Dual-feed A/B sequence arbitration.
3. OpenOnload/ef_vi integration behind compile-time feature gates.
4. True AVX-512 microstructure implementation and benchmark harness.
5. PTP/hardware timestamp calibration and latency histograms.
6. FPGA TOE/risk timing closure at 322.265 MHz.
7. CI, packet replay and hardware-in-the-loop verification.

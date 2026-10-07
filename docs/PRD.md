# PHOTON APEX-VELOX GATE — Product Requirements

## Objective
Build a deterministic, hardware-accelerated tick-to-trade engine with a measured sub-100 ns software/hardware path where the deployed NIC, FPGA, CPU, clocking and exchange path support it.

## Pipeline
PHY multicast ingress → Feed A/B arbitration → microstructure signal → FPGA pre-trade risk → OUCH serialization → kernel-bypass egress → telemetry.

## Requirements
- Zero dynamic allocation on hot path.
- Preallocated, cache-line aligned rings.
- OUCH 5.0 wire serialization with explicit endianness.
- Dual-feed sequence tracking and duplicate suppression.
- AVX-512-capable signal path.
- FPGA risk checks: quantity, price floor, price ceiling, notional.
- FIX/OUCH/session abstractions with secret-free configuration.
- TSC and hardware timestamp telemetry; PTP offset monitoring.
- CI smoke tests plus FPGA self-checking simulation.
- Production claims must be backed by hardware measurements, not source-level estimates.

## Safety gates
No live order transmission until protocol conformance, risk-gate simulation, packet capture validation, timestamp calibration and broker/exchange certification are complete.

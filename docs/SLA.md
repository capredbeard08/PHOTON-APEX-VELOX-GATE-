# PHOTON APEX-VELOX GATE — Engineering SLA

Targets are engineering objectives, not guarantees.

| Metric | Target |
|---|---:|
| Software hot-path allocation | 0 |
| OUCH fixed-message serialization | < 12 ns target |
| Risk gate decision | 1 FPGA cycle target |
| Clock | 322.265 MHz target |
| End-to-end tick-to-wire | < 100 ns target |
| PTP offset alert | ±10 ns |
| p50/p90/p99/p99.99 | continuously measured |

A target is accepted only after hardware timestamping, repeated runs, and documented test conditions demonstrate it.

# PHOTON networking path

## ef_vi
The optional ef_vi adapter exposes registered-buffer transmission through ef_vi_transmit() and ef_vi_transmit_push(). The hot path owns pre-registered DMA buffers; no allocation occurs during submission.

## OpenOnload
The optional OpenOnload adapter uses the delegated-send API for Onload-managed TCP connections. Onload prepares Ethernet/IP/TCP headers and the application sends those headers plus payload through ef_vi, then reports transmitted bytes back to Onload so retransmission and ACK state remain correct.

Build with -DPHOTON_ENABLE_EF_VI=ON and/or -DPHOTON_ENABLE_OPENONLOAD=ON only on hosts where the corresponding vendor headers and libraries are installed. Never commit vendor libraries or credentials.

## A/B multicast
Feed arbitration is separated from NIC setup. Each feed can publish sequence numbers into the single-consumer hot path. Equal or stale sequences are suppressed; a sequence jump is marked as a gap so recovery can be triggered off the hot path.

## Hardware timestamps
Use Solarflare hardware TX/RX timestamping and Linux SO_TIMESTAMPING/PTP for wire-to-wire measurement. Software TSC samples remain useful for stage-local profiling but are not a substitute for NIC timestamps.

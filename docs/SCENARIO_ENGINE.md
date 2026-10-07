# PHOTON Multi-Timeframe Scenario Engine

The engine implements the requested “simulate, resimulate, then cross-check” concept without claiming certainty.

## Pipeline

1. **Reconstruct** historical market states from timestamped data.
2. **Condition** on the current microstructure state.
3. **Retrieve analogues** from each horizon (1 ms through 1 s in the current low-latency profile).
4. **Simulate multiple candidate paths**: down, flat and up.
5. **Resimulate** by repeatedly evaluating the nearest historical analogues rather than trusting one nearest chart.
6. **Select the modal outcome** only when its probability separates from the alternatives.
7. **Cross-validate** the winner across all configured horizons.
8. Require consensus and execution-cost checks before an order is eligible.

The storage layer is deliberately fixed-size and allocation-free in the inference path.

## Important distinction

The engine does not “look into the future.” It estimates conditional outcome probabilities from historical analogues and current state. If the analogue set disagrees, the correct output is **NO TRADE**, not a fabricated prediction.

The current live profile uses 1, 5, 10, 50, 100, 500 and 1000 ms horizons. The research pipeline should additionally generate 1-second through 1-minute bars/feature windows so that slower context can veto or reinforce the low-latency decision.

## Research requirements

Training/research must use event-time joins, purged walk-forward splits, embargo periods, realistic queue/execution costs, and no future leakage. Similarity weights and thresholds are model parameters to be learned out-of-sample.

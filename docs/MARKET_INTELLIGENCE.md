# PHOTON Market Intelligence

PHOTON does not use or require material non-public information. The intelligence layer is designed for lawful public, exchange-licensed, or otherwise authorized data.

## Inputs
Full-depth order-book state, trades/aggressor flow, add/cancel activity, cross-venue lead/lag, related-asset returns, short-horizon momentum/volatility, spread, activity and feed staleness.

## Decision model
Features are transformed into a versioned logistic inference model. The output is a probability, expected move, explicit transaction-cost estimate, executable edge and confidence. A trade is emitted only when both edge and confidence thresholds are satisfied.

## No fake certainty
The default coefficients are a transparent baseline, not a claim of profitable training. Production coefficients must be fitted offline from licensed historical data using purged walk-forward validation, transaction-cost-aware labels and strict out-of-sample evaluation.

## Data-plane architecture
Feed handlers publish immutable snapshots into the hot path. The intelligence engine is deterministic and allocation-free. Slow research jobs train/recalibrate models offline; only a compact versioned coefficient vector is loaded by the trading process.

## Information hierarchy
1. Direct exchange data and full-depth order flow
2. Cross-venue information and related instruments
3. Authorized real-time event/news feeds
4. Historical/derived features
5. Model inference
6. Execution-cost and risk gating

No component is allowed to fabricate data, leak future observations, or use unverified "signals".

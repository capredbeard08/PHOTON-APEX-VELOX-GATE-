# PHOTON Sandbox

The sandbox is an isolated replay/backtest environment. It never submits live
orders. Its purpose is to answer: **"What would PHOTON have done here?"**

## Pipeline

Historical market data -> snapshot reconstruction -> scenario simulation ->
cross-timeframe validation -> decision ledger.

The scenario hierarchy is:

1s, 5s, 10s, 15s, 30s, 1m, 5m, 15m, 30m, 1h, 4h, 12h, 1d, 3d,
1w, 2w, 1mo, 3mo, 6mo, 1y.

The sandbox must report every timeframe, the winning scenario, probability,
path agreement, regime agreement, and whether the final outcome was executable.

**Safety:** this environment has no broker/session/order-routing dependency.
It is deliberately incapable of submitting an order.

Use historical data with strict point-in-time joins and purged walk-forward
evaluation. Never train on the future portion of a replay window.

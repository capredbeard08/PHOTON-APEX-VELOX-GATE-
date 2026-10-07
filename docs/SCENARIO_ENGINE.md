# Multi-Timeframe Scenario Engine

PHOTON's scenario engine is a historical-analogue and path-consensus layer. It is not a crystal ball and it never treats a single model prediction as certainty.

## Research horizon

The standard consensus hierarchy spans:

- 1s, 5s, 10s, 15s, 30s
- 1m, 5m, 15m, 30m
- 1h, 4h, 12h
- 1d, 3d
- 1w, 2w
- 1mo, 3mo, 6mo
- 1y

simulate() also accepts arbitrary horizons, so this hierarchy is a default research grid rather than a hard limit.

## What happens at each horizon

1. Build the current market-state signature from depth, flow, cancellations, pressure, momentum, volatility, spread and related-market information.
2. Retrieve the closest historical states for the same horizon.
3. Re-simulate their observed continuation paths rather than looking only at the final return.
4. Separate the paths into down / flat / up outcomes.
5. Weight analogues by state similarity.
6. Measure terminal return, maximum favourable excursion, maximum adverse excursion and path consistency.
7. Re-test the dominant outcome against every other populated timeframe.

The result is therefore a scenario survivor, not simply a majority vote.

## Cross-timeframe gate

A candidate must survive three independent checks:

- endpoint agreement — the directional outcome remains supported;
- path agreement — simulated continuation shapes remain compatible;
- regime agreement — the result is supported by both fast execution horizons and slower market-regime horizons.

If those checks disagree, the correct result is executable=false / NO TRADE.

## Historical data requirements

The live C++ process intentionally keeps a bounded analogue cache. The complete 1s-to-1y historical archive belongs in an offline research/data service.

Training and evaluation must use:

- event-time aligned market data;
- no future leakage;
- purged walk-forward splits with embargo;
- transaction-cost and slippage assumptions;
- out-of-sample evaluation;
- versioned datasets and model coefficients;
- separate research and live-execution paths.

For long horizons, the system should use OHLCV plus volume/volatility/regime features and, where available, higher-quality depth/flow history. Short horizons should retain L2/L3/order-flow detail.

The engine must be allowed to conclude NO TRADE. A dominant simulated path is only actionable when it remains stable across the requested timeframe hierarchy.

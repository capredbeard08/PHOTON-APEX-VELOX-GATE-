#!/usr/bin/env python3
import csv
import math
import statistics
import sys

def percentile(xs, p):
    if not xs:
        return float("nan")
    ys = sorted(xs)
    k = (len(ys) - 1) * p
    lo, hi = math.floor(k), math.ceil(k)
    return ys[lo] if lo == hi else ys[lo] + (ys[hi] - ys[lo]) * (k - lo)

def report(name, xs):
    if not xs:
        return
    print(f"{name}: count={len(xs)} p50={percentile(xs,.50):.3f}ns "
          f"p90={percentile(xs,.90):.3f}ns p99={percentile(xs,.99):.3f}ns "
          f"p99.99={percentile(xs,.9999):.3f}ns mean={statistics.mean(xs):.3f}ns "
          f"stdev={statistics.pstdev(xs):.3f}ns min={min(xs):.3f}ns max={max(xs):.3f}ns "
          f"jitter={max(xs)-min(xs):.3f}ns")

def main(path):
    with open(path, newline="") as f:
        rows = list(csv.DictReader(f))
    if not rows:
        raise SystemExit("empty CSV")

    if "latency_ns" in rows[0]:
        report("wire_to_wire", [float(r["latency_ns"]) for r in rows])
    elif "t0" in rows[0] and "t4" in rows[0]:
        for i, j, name in [
            ("t0", "t1", "ingress_to_signal"),
            ("t1", "t2", "signal_to_risk"),
            ("t2", "t3", "risk_to_ouch"),
            ("t3", "t4", "ouch_to_wire"),
            ("t0", "t4", "wire_to_wire"),
        ]:
            report(name, [float(r[j]) - float(r[i]) for r in rows])
    else:
        raise SystemExit("CSV must contain latency_ns or t0..t4 columns")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("usage: analyze_hft_latency.py latency.csv")
    main(sys.argv[1])

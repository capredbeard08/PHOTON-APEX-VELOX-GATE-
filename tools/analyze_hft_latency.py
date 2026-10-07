#!/usr/bin/env python3
import csv, math, statistics, sys

def percentile(xs, p):
    if not xs: return float("nan")
    ys=sorted(xs); k=(len(ys)-1)*p; lo=math.floor(k); hi=math.ceil(k)
    return ys[lo] if lo==hi else ys[lo]+(ys[hi]-ys[lo])*(k-lo)

def main(path):
    rows=[]
    with open(path,newline="") as f:
        for r in csv.DictReader(f):
            if "latency_ns" in r: rows.append(float(r["latency_ns"]))
            elif all(k in r for k in ("t0","t4")): rows.append(float(r["t4"])-float(r["t0"]))
    if not rows: raise SystemExit("no latency_ns or t0/t4 columns")
    print(f"count={len(rows)}")
    for p in (.50,.90,.99,.9999): print(f"p{p*100:g}={percentile(rows,p):.3f} ns")
    print(f"mean={statistics.mean(rows):.3f} ns stdev={statistics.pstdev(rows):.3f} ns")
    print(f"min={min(rows):.3f} ns max={max(rows):.3f} ns")

if __name__=="__main__":
    if len(sys.argv)!=2: raise SystemExit("usage: analyze_hft_latency.py latency.csv")
    main(sys.argv[1])

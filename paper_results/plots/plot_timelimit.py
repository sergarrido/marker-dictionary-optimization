#!/usr/bin/env python3
"""Effect of the per-solve time limit on the proposed suboptimal (packing) method
for 6x6 markers: inter-marker distance (timelimit_dist_6x6.png) and generation
time in log scale (timelimit_time_6x6.png) vs dictionary size, one curve per time
limit. Data: timelimit/tl<L>/6x6 for 10/50/100 s; the 150 s curve is the standard
proposed_suboptimal/6x6 run (the one used in the other figures)."""
import os
import matplotlib.pyplot as plt
from _common import LINEWIDTH, FIGS, load_metric, METHODS

N = 6
# label, results folder, colour, linestyle. 150 s reuses the Proposed Suboptimal
# style (tab:blue solid) since it is the same run shown elsewhere.
LIMITS = [
    ("10",  "timelimit/tl10",      "tab:orange", "--"),
    ("50",  "timelimit/tl50",      "tab:green",  "-."),
    ("100", "timelimit/tl100",     "tab:red",    ":"),
    ("150", "proposed_suboptimal", "tab:blue",   "-"),
]


def dist_ylim():
    """Match the y-range of the normal dist_6x6 figure (auto-scaled over all
    methods, with matplotlib's default 5% margin)."""
    lo = hi = None
    for method, _, _, _ in METHODS:
        d = load_metric(method, N, "dist")
        if not d:
            continue
        ys = d[1]
        lo = min(ys) if lo is None else min(lo, min(ys))
        hi = max(ys) if hi is None else max(hi, max(ys))
    if lo is None:
        return None
    m = 0.05 * (hi - lo)
    return (lo - m, hi + m)


def panel(metric, ylabel, out, logy=False, legend_loc="lower right", ylim=None):
    plt.figure(figsize=(10, 6))
    plotted = 0
    for label, method, color, ls in LIMITS:
        d = load_metric(method, N, metric)
        if not d:
            continue
        xs, ys, _ = d
        plt.plot(xs, ys, linestyle=ls, color=color, linewidth=LINEWIDTH,
                 label=f"time limit {label}s")
        plotted += 1
    if not plotted:
        plt.close()
        print(f"skip {out}: no data")
        return
    plt.xlabel(r"Dictionary size $p$")
    plt.ylabel(ylabel)
    plt.xlim(0, 500)
    if ylim is not None:
        plt.ylim(ylim)
    if logy:
        plt.yscale("log")
        plt.grid(True, which="major", alpha=0.3)
        plt.grid(True, which="minor", alpha=0.15)
    else:
        plt.grid(True, alpha=0.3)
    plt.legend(loc=legend_loc)
    plt.tight_layout()
    plt.savefig(os.path.join(FIGS, out), dpi=300)
    plt.close()
    print(f"wrote {out}")


def main():
    os.makedirs(FIGS, exist_ok=True)
    panel("dist", r"Inter-marker distance $\tau$", "timelimit_dist_6x6.png", legend_loc="upper right", ylim=dist_ylim())
    panel("times", "Generation time (s)", "timelimit_time_6x6.png", logy=True, legend_loc="lower right")


if __name__ == "__main__":
    main()

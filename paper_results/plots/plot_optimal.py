#!/usr/bin/env python3
"""Proposed Optimal method: inter-marker distance (opt_dist_combined.png) and
generation time (opt_time_combined.png) vs dictionary size, for 3x3 and 4x4,
each as its own figure (combined side by side in the paper). Data from
paper_results/proposed_optimal/{3x3,4x4}/run*/{dist.txt,times.txt}."""
import os
import matplotlib.pyplot as plt
from _common import LINEWIDTH, FIGS, load_metric

SERIES = [(3, r"$3\times3$", "-"), (4, r"$4\times4$", "--")]


def panel(metric, ylabel, out, ylim_top=None, logy=False):
    plt.figure(figsize=(10, 6))
    plotted = 0
    for N, label, ls in SERIES:
        data = load_metric("proposed_optimal", N, metric)
        if not data:
            continue
        xs, ys, _ = data
        plt.plot(xs, ys, linestyle=ls, linewidth=LINEWIDTH, label=label)
        plotted += 1
    if not plotted:
        plt.close()
        print(f"skip {out}: no data")
        return
    plt.xlabel(r"Dictionary size $p$")
    plt.ylabel(ylabel)
    plt.xlim(left=0)
    if ylim_top is not None:
        plt.ylim(top=ylim_top)
    if logy:
        plt.yscale("log")
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(FIGS, out), dpi=300)
    plt.close()
    print(f"wrote {out}")


def main():
    os.makedirs(FIGS, exist_ok=True)
    panel("dist", r"Inter-marker distance $\tau$", "opt_dist_combined.png", ylim_top=12)
    panel("times", "Generation time (s)", "opt_time_combined.png")


if __name__ == "__main__":
    main()

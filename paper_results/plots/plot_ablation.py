#!/usr/bin/env python3
"""Ablation of the compact-packing objective: inter-marker distance vs dictionary
size for the Proposed Suboptimal method with the compact-packing objective
(compact packing) and with it removed, returning any feasible marker (feasible
only). One figure per marker size: ablation_dist_<NxN>.png (in figures/).
Data: proposed_suboptimal/ (compact) and proposed_suboptimal_feasible/."""
import os
import matplotlib.pyplot as plt
from _common import LINEWIDTH, FIGS, load_metric

SERIES = [("proposed_suboptimal", "Compact packing", "tab:blue", "-"),
          ("proposed_suboptimal_feasible", "Feasible only", "tab:red", (0, (5, 2)))]


def main():
    os.makedirs(FIGS, exist_ok=True)
    for N in [4, 5, 6, 7]:
        plt.figure(figsize=(10, 6))
        plotted = 0
        for folder, label, color, ls in SERIES:
            data = load_metric(folder, N, "dist")
            if not data:
                continue
            xs, ys, _ = data
            plt.plot(xs, ys, linestyle=ls, color=color, linewidth=LINEWIDTH, label=label)
            plotted += 1
        if not plotted:
            plt.close()
            continue
        plt.xlabel(r"Dictionary size $p$")
        plt.ylabel(r"Inter-marker distance $\tau$")
        plt.title(rf"${N}\times{N}$")
        plt.xlim(0, 500)
        plt.grid(True, alpha=0.3)
        plt.legend()
        plt.tight_layout()
        out = os.path.join(FIGS, f"ablation_dist_{N}x{N}.png")
        plt.savefig(out, dpi=300)
        plt.close()
        print(f"wrote {out}")


if __name__ == "__main__":
    main()

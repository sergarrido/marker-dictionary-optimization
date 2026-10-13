#!/usr/bin/env python3
"""Run-to-run variation: standard deviation of the inter-marker distance across
the runs."""
import os
import matplotlib.pyplot as plt
from _common import METHODS, LINEWIDTH, FIGS, load_metric


def main():
    os.makedirs(FIGS, exist_ok=True)
    for N in [4, 5, 6, 7]:
        plt.figure(figsize=(10, 3))
        plotted = 0
        for method, label, color, ls in METHODS:
            data = load_metric(method, N, "dist")
            if not data:
                continue
            xs, _, std = data
            plt.plot(xs, std, linestyle=ls, color=color, linewidth=LINEWIDTH, label=label)
            plotted += 1
        if not plotted:
            plt.close()
            continue
        plt.xlabel(r"Dictionary size $p$")
        plt.ylabel(r"Standard deviation of $\tau$", fontsize=13)
        plt.title(rf"${N}\times{N}$")
        plt.xlim(0, 500)
        plt.ylim(-0.03, 1)
        plt.grid(True, alpha=0.3)
        plt.legend(ncol=3, loc="upper center", fontsize=11)
        plt.tight_layout()
        out = os.path.join(FIGS, f"std_{N}x{N}.png")
        plt.savefig(out, dpi=300, bbox_inches="tight")
        plt.close()
        print(f"wrote {out}")


if __name__ == "__main__":
    main()

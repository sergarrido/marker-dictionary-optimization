#!/usr/bin/env python3
"""Generation time (log scale) vs dictionary size, one curve per method, one
figure per marker size: time_4x4.png ... time_7x7.png (in figures/)."""
import os
import matplotlib.pyplot as plt
from _common import METHODS, LINEWIDTH, FIGS, load_metric


def main():
    os.makedirs(FIGS, exist_ok=True)
    for N in [4, 5, 6, 7]:
        plt.figure(figsize=(10, 6))
        plotted = 0
        for method, label, color, ls in METHODS:
            data = load_metric(method, N, "times")
            if not data:
                continue
            xs, ys, _ = data
            plt.plot(xs, ys, linestyle=ls, color=color, linewidth=LINEWIDTH, label=label)
            plotted += 1
        if not plotted:
            plt.close()
            continue
        plt.xlabel(r"Dictionary size $p$")
        plt.ylabel("Generation time (s)")
        plt.title(rf"${N}\times{N}$")
        plt.yscale("log")
        plt.xlim(0, 500)
        plt.grid(True, which="major", alpha=0.3)
        plt.grid(True, which="minor", alpha=0.15)
        plt.legend(loc="lower right")
        plt.tight_layout()
        out = os.path.join(FIGS, f"time_{N}x{N}.png")
        plt.savefig(out, dpi=300)
        plt.close()
        print(f"wrote {out}")


if __name__ == "__main__":
    main()

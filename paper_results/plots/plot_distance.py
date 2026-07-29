#!/usr/bin/env python3
"""Inter-marker distance vs dictionary size, one curve per method, one figure
per marker size: dist_4x4.png ... dist_7x7.png (in figures/)."""
import os
import matplotlib.pyplot as plt
from _common import METHODS, LINEWIDTH, FIGS, load_metric


def main():
    os.makedirs(FIGS, exist_ok=True)
    for N in [4, 5, 6, 7]:
        plt.figure(figsize=(10, 6))
        plotted = 0
        for method, label, color, ls in METHODS:
            data = load_metric(method, N, "dist")
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
        out = os.path.join(FIGS, f"dist_{N}x{N}.png")
        plt.savefig(out, dpi=300)
        plt.close()
        print(f"wrote {out}")


if __name__ == "__main__":
    main()

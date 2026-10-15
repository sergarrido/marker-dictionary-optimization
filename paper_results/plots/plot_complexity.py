#!/usr/bin/env python3
"""Marker complexity (bit-transition rate) of the generated dictionaries.
For each marker size, a box plot per method of the fraction of adjacent cell
pairs (horizontal + vertical) with different values, pooled over the first 500
markers of each run's dictionary across all runs.
Output: complexity_combined.png (in figures/)."""
import os, glob
import numpy as np
import matplotlib.pyplot as plt
from _common import FIGS, METHODS, parse_set

# (folder, short label); fixed order, methods present at every marker size.
PANEL = [("proposed_suboptimal", "Proposed"), ("milp2016", "MILP"),
         ("greedy2022", "Greedy"), ("aruco2014", "ArUco"),
         ("apriltag2011", "AprilTag")]
COLOR = {folder: color for folder, _, color, _ in METHODS}
RESULTS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def gather_markers(method, N):
    """Pool the markers of the generated dictionaries over all runs. For each run,
    take its largest dictionary and keep the first 500 markers (so all methods are
    compared at the same dictionary size), then concatenate across runs."""
    chunks = []
    for rundir in sorted(glob.glob(os.path.join(RESULTS, method, f"{N}x{N}", "run*"))):
        best, bn = None, -1
        for f in glob.glob(os.path.join(rundir, "dict*.set")):
            if "incumbent" in f:
                continue
            a = parse_set(f)
            if a.shape[0] > bn:
                bn, best = a.shape[0], a
        if best is not None:
            chunks.append(best[:500])
    return np.concatenate(chunks) if chunks else None


def transition_fraction(a, N):
    g = a.reshape(-1, N, N)
    h = (g[:, :, 1:] != g[:, :, :-1]).sum(axis=(1, 2))
    v = (g[:, 1:, :] != g[:, :-1, :]).sum(axis=(1, 2))
    return (h + v) / (2 * N * (N - 1))


def main():
    os.makedirs(FIGS, exist_ok=True)
    fig, axes = plt.subplots(2, 2, figsize=(11, 7), sharey=True)
    for ax, N in zip(axes.flat, [4, 5, 6, 7]):
        data, labels, colors = [], [], []
        for folder, short in PANEL:
            a = gather_markers(folder, N)
            if a is None:
                continue
            data.append(transition_fraction(a, N))
            labels.append(short)
            colors.append(COLOR[folder])
        bp = ax.boxplot(data, whis=(0, 100), patch_artist=True, widths=0.6,
                        medianprops=dict(color="black"))
        for patch, c in zip(bp["boxes"], colors):
            patch.set_facecolor(c)
            patch.set_alpha(0.55)
        ax.set_xticklabels(labels, rotation=20)
        ax.set_title(rf"${N}\times{N}$")
        ax.grid(True, axis="y", alpha=0.3)
    for ax in axes[:, 0]:
        ax.set_ylabel("Bit-transition rate")
    fig.tight_layout()
    out = os.path.join(FIGS, "complexity_combined.png")
    fig.savefig(out, dpi=300)
    plt.close(fig)
    print(f"wrote {out}")


if __name__ == "__main__":
    main()

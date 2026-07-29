"""Shared style, method table and data loaders for the paper figures.

Data is read from the paper_results tree (the parent of this folder):
    paper_results/<method>/<NxN>/run<k>/dist.txt   (size distance)
    paper_results/<method>/<NxN>/run<k>/times.txt  (size seconds)
    paper_results/<method>/<NxN>/run<k>/dict*.set  (raw dictionaries)
Curves are averaged over the available runs.
"""
import os, glob, re
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

plt.rcParams.update({
    "font.size": 15,
    "axes.labelsize": 18,
    "axes.titlesize": 17,
    "legend.fontsize": 15,
    "xtick.labelsize": 14,
    "ytick.labelsize": 14,
})
LINEWIDTH = 2.4

HERE = os.path.dirname(os.path.abspath(__file__))
RESULTS = os.path.dirname(HERE)
FIGS = os.path.join(HERE, "figures")

# method folder -> (label, color, linestyle); order matters for the legend.
METHODS = [
    ("proposed_optimal_timelimit", "Proposed Optimal (time limit)", "tab:gray",   (0, (5, 2))),
    ("proposed_suboptimal",        "Proposed Suboptimal",           "tab:blue",   "-"),
    ("milp2016",                   "MILP",                          "tab:orange", "--"),
    ("greedy2022",                 "Greedy",                        "tab:green",  "-."),
    ("aruco2014",                  "ArUco",                         "tab:red",    ":"),
    ("apriltag2011",               "AprilTag",                      "tab:purple", (0, (3, 1, 1, 1))),
]


def _read_curve(path):
    xs, ys = [], []
    for line in open(path):
        p = line.split()
        if len(p) >= 2:
            xs.append(int(p[0])); ys.append(float(p[1]))
    return dict(zip(xs, ys))


def load_metric(method, N, metric):
    """Average <metric>.txt (metric = 'dist' or 'times') over all runs of a
    method/size. Returns (xs, mean, std) sorted by size, or None if no data."""
    fname = {"dist": "dist.txt", "times": "times.txt"}[metric]
    runs = sorted(glob.glob(os.path.join(RESULTS, method, f"{N}x{N}", "run*", fname)))
    if not runs:
        return None
    acc = {}
    for r in runs:
        for s, v in _read_curve(r).items():
            acc.setdefault(s, []).append(v)
    xs = sorted(acc)
    if not xs:
        return None
    mean = [float(np.mean(acc[s])) for s in xs]
    std = [float(np.std(acc[s], ddof=1)) if len(acc[s]) > 1 else 0.0 for s in xs]
    return xs, mean, std


def parse_set(f):
    ms = [[int(c) for c in m.group(1)] for line in open(f, errors="ignore")
          for m in [re.search(r'marker_\d+:\s*"([01]+)"', line)] if m]
    return np.array(ms, dtype=np.int16)


def vmat(a, N):
    """Pairwise minimum visual (rotation-aware) Hamming distance matrix."""
    idx = [(N - 1 - x) * N + y for y in range(N) for x in range(N)]
    rs = [a]
    for _ in range(3):
        rs.append(rs[-1][:, idx])
    best = None
    for R in rs:
        H = a @ (1 - R).T + (1 - a) @ R.T
        best = H if best is None else np.minimum(best, H)
    return best


def new_figure():
    plt.figure(figsize=(10, 6))

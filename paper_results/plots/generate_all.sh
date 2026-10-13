#!/bin/bash
# Regenerate all paper figures from the results in paper_results/.
# Figures are written to plots/figures/. Assumes the methods have already been
# run (paper_results/<method>/<NxN>/run<k>/{dist.txt,times.txt,dict*.set}).
cd "$(dirname "$0")"
python3 plot_distance.py
python3 plot_time.py
python3 plot_std.py
python3 plot_optimal.py
python3 plot_timelimit.py

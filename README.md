# Square Fiducial Marker Dictionary Generation

Code accompanying the paper *"Maximizing Inter-Marker Distance in Square Fiducial
Marker Dictionary Generation"*.

It generates square marker dictionaries with several methods and measures their
inter-marker distance and generation time, so they can be compared. Two proposed
methods (optimal, suboptimal) are provided together with reimplementations of
prior methods used as baselines.

| Binary | Method / paper |
|---|---|
| `proposed_optimal` | Proposed optimal — exact max-clique |
| `proposed_suboptimal` | Proposed suboptimal — packing (CP-SAT) |
| `milp2016` | *Generation of fiducial marker dictionaries using MILP* (Pattern Recognition 2016) |
| `greedy2022` | *Reflection-Aware Generation and Identification of Square Marker Dictionaries* (Sensors 2022) |
| `aruco2014` | *Automatic generation and detection of highly reliable fiducial markers…* (Pattern Recognition 2014) |
| `apriltag2011` | *AprilTag: A robust and flexible visual fiducial system* (ICRA 2011) |
| `mindistances` | Utility: distance-vs-size curve of a `.set` dictionary |

## Dependencies

- C++17 compiler and CMake ≥ 3.10
- OpenMP — `aruco2014`, `apriltag2011`
- GMP (`libgmp`, `libgmpxx`) — `apriltag2011`
- OR-Tools — `milp2016`, `proposed_suboptimal` (prebuilt in `third_party/ortools`)

If OR-Tools is not present, those two methods are skipped and the rest still build.

### Installing OR-Tools

Download it into `third_party/`
(v9.11.4210, Ubuntu 20.04 build used here):

```sh
mkdir -p third_party && cd third_party
curl -sSL -o ortools.tar.gz \
  https://github.com/google/or-tools/releases/download/v9.11/or-tools_amd64_ubuntu-20.04_cpp_v9.11.4210.tar.gz
tar xzf ortools.tar.gz && rm ortools.tar.gz
mv or-tools_*/ ortools
```

This must leave `third_party/ortools/{include,lib,...}`; CMake detects it there
automatically. For other platforms pick the matching archive from the
[OR-Tools releases](https://github.com/google/or-tools/releases).

## Compiling

```sh
mkdir build && cd build
cmake ..
make
```

Binaries are left in `build/`.

## Running the methods

Each method has a script in `paper_results/`. Parameters are set at the **top of the
script** (marker sizes, dictionary size, number of runs, time limit, binary
paths) — there is nothing to pass on the command line. Run them from `paper_results/`:

```sh
cd paper_results
./run_proposed_optimal_timelimit.sh
./run_proposed_suboptimal.sh
./run_milp2016.sh
./run_greedy2022.sh
./run_aruco2014.sh
./run_apriltag2011.sh
./run_proposed_optimal.sh          # exact optimum, only 3x3/4x4, huge time limit
```

Each run writes into `paper_results/<method>/<NxN>/run<k>/`:

- `dict*.set` — the generated dictionary/dictionaries
- `times.txt` — generation time per dictionary size
- `dist.txt` — inter-marker distance per dictionary size

To average distances and times across the runs of a size, giving mean and
standard deviation per dictionary size:

```sh
./aggregate.sh <method>/<NxN>      # e.g. ./aggregate.sh proposed_suboptimal/4x4
```

This writes `dist_stats.txt` and `times_stats.txt` in that folder.

## Generating the paper figures

Recommended first: recompute every `dist.txt` straight from the dictionaries, so
the figures never rely on a stale or partially-written `dist.txt` (needs
`build/mindistances`):

```sh
cd paper_results
./recompute_distances.sh .        # rebuilds run*/dist.txt from the .set files
```

Then produce the figures with the scripts in `paper_results/plots/`, which read
the results directly from `paper_results/`:

```sh
cd plots
./generate_all.sh
```

Figures are written to `paper_results/plots/figures/`:

- `dist_<NxN>.png` — inter-marker distance vs dictionary size, one curve per method
- `time_<NxN>.png` — generation time vs dictionary size
- `opt_dist_combined.png`, `opt_time_combined.png` — proposed optimal method (3×3, 4×4)
- `timelimit_dist_6x6.png`, `timelimit_time_6x6.png` — effect of the per-solve time
  limit on the proposed suboptimal method (6×6).

Each script can also be run on its own (`plot_distance.py`, `plot_time.py`,
`plot_optimal.py`, `plot_timelimit.py`). Marker sizes with no data are skipped.

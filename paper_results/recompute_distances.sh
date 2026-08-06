#!/bin/bash
# Recompute dist.txt for every run from its dictionaries (dict*.set), ignoring any
# existing dist.txt. Uses accumulate_distances.sh (mindistances + envelope), which
# handles both single-.set methods (mip/aruco/apriltag) and per-distance .set
# methods (packing/greedy/optimal) uniformly. Note: times.txt is a generation-time
# record and is NOT recomputed (it cannot be derived from the .set files).
#
# Usage: recompute_distances.sh <folder>   (default: current dir; recurses into run*/)
# --- parameters ---
MINDIST=../build/mindistances        # distances binary
ACC=./accumulate_distances.sh        # aggregation script
# ------------------

cd "$(dirname "$0")"   # anchor to paper_results/ so the relative paths above work

if [ ! -x "$MINDIST" ]; then
    echo "Error: $MINDIST not found. Build it first:  (cd .. && mkdir -p build && cd build && cmake .. && make)" >&2
    exit 1
fi

DIR="${1:-.}"

find "$DIR" -type d -name 'run*' | sort | while read -r run; do
    ls "$run"/dict*.set >/dev/null 2>&1 || continue
    echo "== $run =="
    "$ACC" "$MINDIST" "$run/dict" "$run/dist.txt"
done

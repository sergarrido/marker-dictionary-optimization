#!/bin/bash
# Ablation of the compact-packing objective: same binary, constraints, solver and
# time limit as run_proposed_suboptimal.sh, but with FEASIBLE_ONLY=1 so each
# insertion step returns any feasible marker instead of the most compact one.
# --- parameters ---
BIN=../build/proposed_suboptimal     # method binary (built from the same source)
MINDIST=../build/mindistances        # distances binary
ACC=./accumulate_distances.sh # distance-accumulation script
OUT=proposed_suboptimal_feasible     # output base dir
RUN_INI=1
RUN_END=1
SIZES="4 5 6 7"
DICTIONARY_TARGET=500
TIMELIMIT=150
# ------------------

for n in $SIZES; do
    for r in $(seq $RUN_INI $RUN_END); do
        dir="$OUT/${n}x${n}/run$r"
        if [ -d "$dir" ]; then echo "== skip proposed_suboptimal_feasible ${n}x${n} run$r (already exists) =="; continue; fi
        mkdir -p "$dir"
        echo "== proposed_suboptimal_feasible ${n}x${n} run$r =="
        FEASIBLE_ONLY=1 "$BIN" --n $n --target $DICTIONARY_TARGET --seed $r --timelimit $TIMELIMIT \
               --out-prefix "$dir/dict" --times "$dir/times.txt"
        "$ACC" "$MINDIST" "$dir/dict" "$dir/dist.txt"
    done
done

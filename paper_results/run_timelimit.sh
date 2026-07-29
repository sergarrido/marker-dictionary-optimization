#!/bin/bash
# Proposed Suboptimal on 6x6 for several per-solve time limits (for the
# timelimit_6x6 figure). Each time limit L writes into
# timelimit/tl<L>/6x6/run<k>/.
# --- parameters ---
BIN=../build/proposed_suboptimal     # method binary
MINDIST=../build/mindistances        # distances binary
ACC=./accumulate_distances.sh        # distance-accumulation script
N=6                                  # marker size (6x6)
DICTIONARY_TARGET=500
RUN_INI=1
RUN_END=5
TIMELIMITS="10 50 100"   # 150 s is the standard run_proposed_suboptimal.sh output
# ------------------

for tl in $TIMELIMITS; do
    for r in $(seq $RUN_INI $RUN_END); do
        dir="timelimit/tl${tl}/${N}x${N}/run$r"
        if [ -d "$dir" ]; then echo "== skip tl${tl} ${N}x${N} run$r (already exists) =="; continue; fi
        mkdir -p "$dir"
        echo "== proposed_suboptimal tl${tl} ${N}x${N} run$r =="
        "$BIN" --n $N --target $DICTIONARY_TARGET --seed $r --timelimit $tl \
               --out-prefix "$dir/dict" --times "$dir/times.txt"
        "$ACC" "$MINDIST" "$dir/dict" "$dir/dist.txt"
    done
done

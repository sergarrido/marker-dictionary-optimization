#!/bin/bash
# --- parameters ---
BIN=../build/proposed_suboptimal     # method binary
MINDIST=../build/mindistances        # distances binary
ACC=./accumulate_distances.sh # distance-accumulation script
OUT=proposed_suboptimal              # output base dir
RUN_INI=1
RUN_END=5
SIZES="4 5 6 7"
DICTIONARY_TARGET=500
TIMELIMIT=150
# ------------------

for n in $SIZES; do
    for r in $(seq $RUN_INI $RUN_END); do
        dir="$OUT/${n}x${n}/run$r"
        if [ -d "$dir" ]; then echo "== skip proposed_suboptimal ${n}x${n} run$r (already exists) =="; continue; fi
        mkdir -p "$dir"
        echo "== proposed_suboptimal ${n}x${n} run$r =="
        "$BIN" --n $n --target $DICTIONARY_TARGET --seed $r --timelimit $TIMELIMIT \
               --out-prefix "$dir/dict" --times "$dir/times.txt"
        "$ACC" "$MINDIST" "$dir/dict" "$dir/dist.txt"
    done
done

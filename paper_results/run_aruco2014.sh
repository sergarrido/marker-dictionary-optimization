#!/bin/bash
# --- parameters ---
BIN=../build/aruco2014        # method binary
MINDIST=../build/mindistances # distances binary
OUT=aruco2014                 # output base dir
RUN_INI=1
RUN_END=5
SIZES="4 5 6 7"
DICTIONARY_TARGET=500
TIMELIMIT=150
# ------------------

for n in $SIZES; do
    for r in $(seq $RUN_INI $RUN_END); do
        dir="$OUT/${n}x${n}/run$r"
        if [ -d "$dir" ]; then echo "== skip aruco2014 ${n}x${n} run$r (already exists) =="; continue; fi
        mkdir -p "$dir"
        echo "== aruco2014 ${n}x${n} run$r =="
        "$BIN" --n $n --target $DICTIONARY_TARGET --seed $r --timelimit $TIMELIMIT \
               --out "$dir/dict.set" --times "$dir/times.txt"
        "$MINDIST" "$dir/dict.set" "$dir/dist.txt"
    done
done

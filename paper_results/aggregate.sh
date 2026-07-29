#!/bin/bash
# Given a folder with run1/ run2/ ... subdirs, aggregate their per-size curves
# into mean + standard deviation across runs. Produces two files in that folder:
#   dist_stats.txt  <- from each run's dist.txt
#   times_stats.txt <- from each run's times.txt
# Each output line: "size mean std nruns".
#
# Usage: aggregate.sh <folder-with-runs>

DIR="$1"

if [ -z "$DIR" ] || [ ! -d "$DIR" ]; then
    echo "Usage: $0 <folder-with-runs>"
    exit 1
fi

aggregate() {
    name="$1"; out="$2"
    files=()
    for f in "$DIR"/run*/"$name"; do [ -f "$f" ] && files+=("$f"); done
    if [ ${#files[@]} -eq 0 ]; then
        echo "No $name found under $DIR/run*/"
        return
    fi
    awk '
    { s[$1]+=$2; ss[$1]+=$2*$2; n[$1]++ }
    END {
        for (k in n) {
            m = s[k]/n[k]
            if (n[k] > 1) { var=(ss[k]-n[k]*m*m)/(n[k]-1); if(var<0)var=0; sd=sqrt(var) }
            else sd = 0
            printf "%s %g %g %d\n", k, m, sd, n[k]
        }
    }' "${files[@]}" | sort -n > "$out"
    echo "Wrote $out (${#files[@]} runs)"
}

aggregate dist.txt  "$DIR/dist_stats.txt"
aggregate times.txt "$DIR/times_stats.txt"

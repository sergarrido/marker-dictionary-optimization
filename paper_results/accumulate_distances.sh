#!/bin/bash
# Accumulate the per-dictionary distance curves produced from a prefix into a
# single distance-vs-size curve. Some methods write one .set per clique (one per
# inter-marker distance); this runs mindistances on each of them and, for every
# dictionary size, keeps the value from the dictionary with the FEWEST markers
# that still reaches that size (which generally has the largest inter-marker
# distance, i.e. the distance-vs-size envelope).
#
# Usage: accumulate_distances.sh <mindistances_binary> <dict_prefix> <output>

set -e

if [ $# -lt 3 ]; then
    echo "Usage: $0 <metric_binary> <dict_prefix> <output>"
    exit 1
fi

METRIC="$1"
PREFIX="$2"
OUT="$3"

TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

shopt -s nullglob
sets=()
for f in "${PREFIX}"*.set; do
    case "$f" in *_incumbent.set) continue ;; esac   # skip live-checkpoint snapshots
    sets+=( "$f" )
done
if [ ${#sets[@]} -eq 0 ]; then
    echo "No .set files found with prefix '${PREFIX}'"
    exit 1
fi

# For each dictionary, emit "size value nmarkers"; nmarkers = its last (largest) size.
for f in "${sets[@]}"; do
    d="$TMPDIR/$(basename "$f").dist"
    "$METRIC" "$f" "$d"
    nf=$(tail -1 "$d" | awk '{print $1}')
    awk -v nf="$nf" '{print $1, $2, nf}' "$d"
done \
    | awk '{ if(!($1 in n) || $3 < n[$1]) { v[$1]=$2; n[$1]=$3 } }
           END { for(s in v) print s, v[s] }' \
    | sort -n > "$OUT"

echo "Wrote $OUT ($(wc -l < "$OUT") sizes, from ${#sets[@]} dictionaries)"

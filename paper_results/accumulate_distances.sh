#!/bin/bash
# Accumulate the per-dictionary distance curves produced from a prefix into a
# single distance-vs-size curve. Some methods write one .set per clique (one per
# inter-marker distance); this runs mindistances on each of them and, for every
# dictionary size, keeps the MAXIMUM distance across all dictionaries reaching
# that size (the distance-vs-size envelope).
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

if ! { [ -x "$METRIC" ] || command -v "$METRIC" >/dev/null 2>&1; }; then
    echo "Error: metric binary '$METRIC' not found or not executable (build it first: cmake .. && make)" >&2
    exit 1
fi

TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

shopt -s nullglob
sets=( "${PREFIX}"*.set )
if [ ${#sets[@]} -eq 0 ]; then
    echo "No .set files found with prefix '${PREFIX}'"
    exit 1
fi

# Run the metric on each dictionary; for every size keep the maximum value.
for f in "${sets[@]}"; do
    d="$TMPDIR/$(basename "$f").dist"
    "$METRIC" "$f" "$d"
    cat "$d"
done \
    | awk '{ if(!($1 in v) || $2 > v[$1]) v[$1]=$2 }
           END { for(s in v) print s, v[s] }' \
    | sort -n > "$OUT"

echo "Wrote $OUT ($(wc -l < "$OUT") sizes, from ${#sets[@]} dictionaries)"

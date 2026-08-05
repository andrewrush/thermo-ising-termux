#!/bin/bash
# wolff_scan.sh — сканирование фазового перехода через Wolff-алгоритм
# Использование: ./wolff_scan.sh [L] [clusters_per_point]

L=${1:-64}
CLUSTERS=${2:-5000}

OUT="wolff_phase_L${L}.csv"
echo "T,<M>,<|M|>,<E>,<M2>,chi,C,flip_pct" > "$OUT"

echo "[*] Wolff scan: L=$L, clusters=$CLUSTERS"
for T in 1.0 1.5 2.0 2.1 2.2 2.25 2.269 2.3 2.4 2.5 3.0 3.5 4.0; do
    echo "    T=$T ..."
    ./wolff_app "$L" "$T" "$CLUSTERS" csv >> "$OUT"
done

echo "[*] Results saved to $OUT"
cat "$OUT"

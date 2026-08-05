#!/bin/bash
# scan_phase.sh — сканирование фазового перехода 2D Ising
# Использование: ./scan_phase.sh [L] [steps_per_point]

L=${1:-64}
STEPS=${2:-5000000}

OUT="phase_L${L}.csv"
echo "T,<M>,<|M|>,<E>,<M2>,chi,C,accept_pct" > "$OUT"

echo "[*] Scanning phase transition: L=$L, steps=$STEPS"
for T in 1.0 1.5 2.0 2.1 2.2 2.25 2.269 2.3 2.4 2.5 3.0 3.5 4.0; do
    echo "    T=$T ..."
    ./ising_app "$L" "$T" "$STEPS" csv >> "$OUT"
done

echo "[*] Results saved to $OUT"
cat "$OUT"

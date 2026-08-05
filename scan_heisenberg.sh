#!/bin/bash
# scan_heisenberg.sh — сканирование фазового перехода Heisenberg
# Использование: ./scan_heisenberg.sh [L] [steps]

L=${1:-16}
STEPS=${2:-2000000}

OUT="phase_heis_L${L}.csv"
echo "T,|M|,<E>,M2,chi,C,accept_pct" > "$OUT"

echo "[*] Heisenberg scan: L=$L, steps=$STEPS, Tc≈1.44"
for T in 0.5 0.8 1.0 1.2 1.3 1.35 1.4 1.44 1.5 1.6 2.0 2.5 3.0; do
    echo "    T=$T ..."
    ./heisenberg_app "$L" "$T" "$STEPS" csv >> "$OUT"
done

echo "[*] Results saved to $OUT"
cat "$OUT"

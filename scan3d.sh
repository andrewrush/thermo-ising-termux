#!/bin/bash
# scan3d.sh — сканирование фазового перехода 3D Изинга
# Использование: ./scan3d.sh [L] [steps]

L=${1:-16}
STEPS=${2:-2000000}

OUT="phase3d_L${L}.csv"
echo "T,<M>,<|M|>,<E>,<M2>,chi,C,accept_pct" > "$OUT"

echo "[*] 3D Ising scan: L=$L, steps=$STEPS, Tc≈4.51"
for T in 3.0 3.5 4.0 4.2 4.4 4.51 4.6 4.8 5.0 5.5 6.0; do
    echo "    T=$T ..."
    ./ising3d_app "$L" "$T" "$STEPS" csv >> "$OUT"
done

echo "[*] Results saved to $OUT"
cat "$OUT"

#!/bin/bash
# parallel_scan.sh — параллельное сканирование фазового перехода
# Использование: ./parallel_scan.sh [L] [steps_per_point]

L=${1:-64}
STEPS=${2:-5000000}
OUT="phase_L${L}_parallel.csv"

echo "[*] Parallel scan: L=$L, steps=$STEPS"
echo "T,<M>,<|M|>,<E>,<M2>,chi,C,accept_pct" > "$OUT"

# Запускаем все температуры параллельно
for T in 1.0 1.5 2.0 2.1 2.2 2.25 2.269 2.3 2.4 2.5 3.0 3.5 4.0; do
    ./ising_app "$L" "$T" "$STEPS" csv >> "$OUT" &
done
wait

# Сортируем по температуре
sort -t',' -k1 -n "$OUT" -o "$OUT"

echo "[*] Results saved to $OUT"
cat "$OUT"

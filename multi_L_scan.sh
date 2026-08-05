#!/bin/bash
# multi_L_scan.sh — сравнение фазовых переходов для разных L
# Использование: ./multi_L_scan.sh [steps]

STEPS=${1:-5000000}

echo "[*] Multi-L phase scan: L=32, 64, 128, steps=$STEPS"
for L in 32 64 128; do
    OUT="phase_L${L}.csv"
    echo ""
    echo "=== L=$L ==="
    ./scan_phase.sh "$L" "$STEPS"
done

echo ""
echo "[*] All scans complete. Results:"
ls -la phase_L*.csv

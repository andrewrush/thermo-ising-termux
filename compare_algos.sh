#!/bin/bash
# compare_algos.sh — сравнение Metropolis vs Wolff
# Использование: ./compare_algos.sh [L] [T]

L=${1:-64}
T=${2:-2.269}

echo "========================================"
echo "  Algorithm comparison: L=$L, T=$T"
echo "========================================"

echo ""
echo "--- Metropolis (10M steps) ---"
termux-wake-lock
./ising_app "$L" "$T" 10000000
echo ""
echo "--- Wolff (100k clusters) ---"
./wolff_app "$L" "$T" 100000
echo ""
termux-wake-unlock

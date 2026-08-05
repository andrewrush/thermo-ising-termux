#!/bin/bash
# ascii_viz.sh — ASCII-визуализация доменов в реальном времени
# Использование: ./ascii_viz.sh [L] [T] [steps]

L=${1:-32}
T=${2:-2.0}
STEPS=${3:-1000000}

echo "[*] Starting ASCII visualization of 2D Ising domains"
echo "    L=$L, T=$T, steps=$STEPS"
echo "    █ = spin +1, ░ = spin -1"
echo "    Press Ctrl+C to stop"
sleep 1

./ising_app "$L" "$T" "$STEPS" ascii

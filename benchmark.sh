#!/bin/bash
# benchmark.sh — сравнение всех моделей
# Использование: ./benchmark.sh

echo "========================================"
echo "  Benchmark: 2D vs 3D vs Heisenberg"
echo "========================================"

echo ""
echo "--- 2D Ising (L=64, T=2.269, 5M) ---"
termux-wake-lock
./ising_app 64 2.269 5000000

echo ""
echo "--- 3D Ising (L=16, T=4.51, 2M) ---"
./ising3d_app 16 4.51 2000000

echo ""
echo "--- Heisenberg (L=16, T=1.44, 2M) ---"
./heisenberg_app 16 1.44 2000000

echo ""
echo "--- GPU 2D Ising (L=64, T=2.269, 1M) ---"
./gpu_app 64 2.269 1000000
termux-wake-unlock

echo ""
echo "[*] Benchmark complete!"

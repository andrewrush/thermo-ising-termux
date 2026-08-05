#!/bin/bash
# hotstart.sh — демонстрация "горячего старта" с физической энтропии
# Использование: ./hotstart.sh

echo "========================================"
echo "  Hot Start: Physical Entropy Demo"
echo "========================================"

# Проверяем termux-sensor
if command -v termux-sensor >/dev/null 2>&1; then
    echo "[*] termux-sensor available. Capturing accelerometer noise..."
    termux-sensor -s Accelerometer -d 100 -n 5
else
    echo "[!] termux-sensor not available (Play Store limitation)."
    echo "    Using /dev/urandom only (SoC HWRNG)."
fi

echo ""
echo "[*] Running 2D Ising with hardware entropy seed..."
./ising_app 32 2.269 1000000

echo ""
echo "[*] Done. Compare with pseudo-random seed if curious."

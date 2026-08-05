#!/bin/bash
# benchmark_power.sh — бенчмарк с энергометрикой
# Использование: ./benchmark_power.sh [L] [steps]

L=${1:-64}
STEPS=${2:-5000000}
TMP="${TMPDIR:-/data/data/com.termux/files/usr/tmp}"

echo "========================================"
echo "  Energy Benchmark: Joules per sample"
echo "========================================"

# Функция чтения мощности
# termux-battery-status может возвращать current в микроамперах или миллиамперах
# Пробуем оба варианта и выбираем разумный
read_power() {
    termux-battery-status 2>/dev/null | awk -F: '
    /voltage/{v=$2}
    /current/{i=$2}
    /temperature/{t=$2}
    END {
        gsub(/[^0-9.-]/,"",v)
        gsub(/[^0-9.-]/,"",i)
        gsub(/[^0-9.-]/,"",t)
        # Если ток > 10000, скорее всего микроамперы
        if (i+0 > 10000) i = i / 1000
        printf "%.0f %.0f %.1f", v, i, t
    }'
}

# 2D Ising
echo ""
echo "--- 2D Ising (L=$L, steps=$STEPS) ---"
read -r V1 I1 T1 <<< "$(read_power)"
echo "[*] Before: V=${V1}mV I=${I1}mA T=${T1}°C"

START=$(date +%s.%N)
./ising_app "$L" 2.269 "$STEPS" > "$TMP/ising_out.txt" 2>&1
END=$(date +%s.%N)

read -r V2 I2 T2 <<< "$(read_power)"
echo "[*] After:  V=${V2}mV I=${I2}mA T=${T2}°C"

TIME=$(echo "$END - $START" | bc)
P1=$(echo "scale=3; $V1 * $I1 / 1000" | bc)
P2=$(echo "scale=3; $V2 * $I2 / 1000" | bc)
PAVG=$(echo "scale=3; ($P1 + $P2) / 2" | bc)
JOULES=$(echo "scale=6; $PAVG * $TIME / 1000" | bc)
JPS=$(echo "scale=9; $JOULES / $STEPS" | bc)

echo "[*] Time: ${TIME}s"
echo "[*] Avg Power: ${PAVG}mW"
echo "[*] Total Energy: ${JOULES}J"
echo "[*] Joules per sample: ${JPS}J"
if [ -f "$TMP/ising_out.txt" ]; then
    tail -n 5 "$TMP/ising_out.txt"
else
    echo "[!] Output file not found"
fi

# 3D Ising
echo ""
echo "--- 3D Ising (L=16, steps=2M) ---"
read -r V1 I1 T1 <<< "$(read_power)"
START=$(date +%s.%N)
./ising3d_app 16 4.51 2000000 > "$TMP/ising3d_out.txt" 2>&1
END=$(date +%s.%N)
read -r V2 I2 T2 <<< "$(read_power)"
TIME=$(echo "$END - $START" | bc)
P1=$(echo "scale=3; $V1 * $I1 / 1000" | bc)
P2=$(echo "scale=3; $V2 * $I2 / 1000" | bc)
PAVG=$(echo "scale=3; ($P1 + $P2) / 2" | bc)
JOULES=$(echo "scale=6; $PAVG * $TIME / 1000" | bc)
JPS=$(echo "scale=9; $JOULES / 2000000" | bc)
echo "[*] Joules per sample: ${JPS}J"
if [ -f "$TMP/ising3d_out.txt" ]; then
    tail -n 3 "$TMP/ising3d_out.txt"
fi

# Heisenberg
echo ""
echo "--- Heisenberg (L=16, steps=2M) ---"
read -r V1 I1 T1 <<< "$(read_power)"
START=$(date +%s.%N)
./heisenberg_app 16 1.44 2000000 > "$TMP/heis_out.txt" 2>&1
END=$(date +%s.%N)
read -r V2 I2 T2 <<< "$(read_power)"
TIME=$(echo "$END - $START" | bc)
P1=$(echo "scale=3; $V1 * $I1 / 1000" | bc)
P2=$(echo "scale=3; $V2 * $I2 / 1000" | bc)
PAVG=$(echo "scale=3; ($P1 + $P2) / 2" | bc)
JOULES=$(echo "scale=6; $PAVG * $TIME / 1000" | bc)
JPS=$(echo "scale=9; $JOULES / 2000000" | bc)
echo "[*] Joules per sample: ${JPS}J"
if [ -f "$TMP/heis_out.txt" ]; then
    tail -n 3 "$TMP/heis_out.txt"
fi

echo ""
echo "[*] Energy benchmark complete!"

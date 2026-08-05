#!/bin/bash
# run_all.sh — полный pipeline: scan -> plot -> git push
# Использование: ./run_all.sh [L] [steps]

L=${1:-64}
STEPS=${2:-5000000}

echo "========================================"
echo "  Full pipeline: L=$L, steps=$STEPS"
echo "========================================"

# 1. Сканирование
echo ""
echo "[1/4] Running phase scan..."
./scan_phase.sh "$L" "$STEPS"

# 2. Построение графиков (если gnuplot установлен)
echo ""
echo "[2/4] Plotting..."
if command -v gnuplot >/dev/null 2>&1; then
    ./plot_phase.sh "phase_L${L}.csv"
else
    echo "[!] gnuplot not installed. Skipping plots."
    echo "    Install: pkg install gnuplot"
fi

# 3. Git add CSV
echo ""
echo "[3/4] Adding results to git..."
git add phase_L*.csv *.sh *.c Makefile README.md .gitignore
git diff --cached --quiet || git commit -m "data: phase scan L=$L, steps=$STEPS"

# 4. Push
echo ""
echo "[4/4] Pushing to GitHub..."
git push

echo ""
echo "[*] Pipeline complete!"

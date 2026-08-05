#!/bin/bash
# fss_analysis.sh — анализ конечно-размерного скалирования
# Использование: ./fss_analysis.sh

echo "========================================"
echo "  Finite-Size Scaling Analysis"
echo "========================================"
echo ""
echo "Theory: Tc(inf) = 2.269, nu = 1, beta = 1/8"
echo ""

for L in 32 64 128; do
    FILE="phase_L${L}.csv"
    if [ ! -f "$FILE" ]; then
        echo "[!] $FILE not found"
        continue
    fi
    echo "--- L=$L ---"
    # Найти T с максимальным chi
    awk -F',' 'NR>1 {if($6>max){max=$6;T=$1;M=$3;C=$7}} END {printf "  Max chi at T=%.3f, |M|=%.3f, C=%.3f, chi=%.2f\n", T, M, C, max}' "$FILE"
done

echo ""
echo "[*] FSS prediction: Tc(L) = Tc(inf) + a*L^(-1/nu)"
echo "    As L -> inf, Tc(L) should approach 2.269 from above"

#!/bin/bash
# plot_multi.sh — сравнительные графики M(T) для L=32,64,128
# Использование: ./plot_multi.sh

if ! command -v gnuplot >/dev/null 2>&1; then
    echo "[!] gnuplot not installed. Run: pkg install gnuplot"
    exit 1
fi

echo "[*] Plotting multi-L comparison..."

for METRIC in 3 6 7; do
    LABEL="M"
    [ "$METRIC" = "6" ] && LABEL="chi"
    [ "$METRIC" = "7" ] && LABEL="C"

    echo ""
    echo "=== $LABEL(T) for L=32,64,128 ==="
    gnuplot -e "
set terminal dumb size 80 25;
set datafile separator ',';
set xlabel 'T';
set ylabel '$LABEL';
set key outside;
plot 'phase_L32.csv' using 1:$METRIC with linespoints title 'L=32', \
     'phase_L64.csv' using 1:$METRIC with linespoints title 'L=64', \
     'phase_L128.csv' using 1:$METRIC with linespoints title 'L=128';
"
done

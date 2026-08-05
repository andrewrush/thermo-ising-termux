#!/bin/bash
# plot_phase.sh — построение графиков фазового перехода
# Использование: ./plot_phase.sh [CSV_FILE]

CSV="${1:-phase_L64.csv}"
if [ ! -f "$CSV" ]; then
    echo "[!] $CSV not found. Run ./scan_phase.sh first."
    exit 1
fi

echo "[*] Plotting phase transition from $CSV..."

# Текстовый график M(T)
echo ""
echo "=== Magnetization M(T) ==="
gnuplot -e "
set terminal dumb size 80 25;
set datafile separator ',';
set key off;
set xlabel 'T';
set ylabel 'M';
plot '$CSV' using 1:3 with linespoints title '<|M|>';
"

# Текстовый график chi(T)
echo ""
echo "=== Susceptibility chi(T) ==="
gnuplot -e "
set terminal dumb size 80 25;
set datafile separator ',';
set key off;
set xlabel 'T';
set ylabel 'chi';
plot '$CSV' using 1:6 with linespoints title 'chi';
"

# Текстовый график C(T)
echo ""
echo "=== Heat Capacity C(T) ==="
gnuplot -e "
set terminal dumb size 80 25;
set datafile separator ',';
set key off;
set xlabel 'T';
set ylabel 'C';
plot '$CSV' using 1:7 with linespoints title 'C';
"

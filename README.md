# thermo-ising-termux

Высокопроизводительный **2D симулятор модели Изинга** для Termux на Android aarch64.
Два алгоритма: **Metropolis** и **Wolff** (кластерный).

## Алгоритмы

| Алгоритм | Файл | Скорость | Крит. замедление | Рекомендация |
|---|---|---|---|---|
| Metropolis | `ising.c` | Быстро (2.5с/10M) | z ≈ 2 | L <= 64 |
| Wolff | `wolff.c` | Медленно (12с/100k) | z ≈ 0 | L >= 64, T ≈ Tc |

## Скрипты

| Скрипт | Описание |
|---|---|
| `scan_phase.sh` | Сканирование T=1..4 через Metropolis |
| `wolff_scan.sh` | Сканирование через Wolff (быстрее для больших L) |
| `parallel_scan.sh` | Параллельный Metropolis-скан |
| `multi_L_scan.sh` | Сканирование для L=32,64,128 |
| `compare_algos.sh` | Сравнение Metropolis vs Wolff |
| `plot_phase.sh` | ASCII-графики для одного L |
| `plot_multi.sh` | Сравнительные графики для разных L |
| `fss_analysis.sh` | Анализ конечно-размерного скалирования |
| `run_all.sh` | Полный pipeline |

## Сборка

```bash
make
```

## Запуск

```bash
# Metropolis (L=64, T=2.269, 10M)
./ising_app

# Wolff (L=64, T=2.269, 5k кластеров)
./wolff_app 64 2.269 5000

# Сравнение алгоритмов
./compare_algos.sh 64 2.269

# Multi-L scan
./multi_L_scan.sh 5000000

# Wolff-скан (рекомендуется для L=128)
./wolff_scan.sh 128 10000

# FSS-анализ
./fss_analysis.sh

# Сравнительные графики
./plot_multi.sh

# Полный pipeline
./run_all.sh 64 5000000
```

## Git

```bash
git add .
git commit -m "feat: your message"
git push
```

# thermo-ising-termux

Высокопроизводительный **2D симулятор модели Изинга** для Termux на Android aarch64.
Два алгоритма: **Metropolis** (локальные обновления) и **Wolff** (кластерные обновления).

## Алгоритмы

| Алгоритм | Файл | Скорость около Tc | Критическое замедление |
|---|---|---|---|
| Metropolis | `ising.c` | Медленно | z ≈ 2 (корреляционное время ~ L^2) |
| Wolff | `wolff.c` | Быстро | z ≈ 0 (корреляционное время ~ const) |

## Особенности

- **2D решётка** LxL с периодическими граничными условиями
- Инициализация через аппаратный RNG SoC (`/dev/urandom`)
- Быстрый XorShift128+ для шагов симуляции
- **Температура батареи** через `termux-battery-status` (обход SELinux)
- **CLI-параметры**: `./ising_app [L] [T] [steps] [csv]` и `./wolff_app [L] [T] [steps] [csv]`
- **Статистика**: `<M>`, `<|M|>`, `<E>`, восприимчивость `chi`, теплоёмкость `C`
- **CSV-режим** для серийных запусков
- **Сканирование фазового перехода** `scan_phase.sh`
- **Параллельное сканирование** `parallel_scan.sh`
- **Визуализация** `plot_phase.sh` через gnuplot
- **Полный pipeline** `run_all.sh`
- **Сравнение алгоритмов** `compare_algos.sh`
- **Multi-L scan** `multi_L_scan.sh` (L=32,64,128)
- Оптимизация под ARM64 NEON (`-march=armv8-a+simd`)

## Сборка

```bash
make
```

## Запуск

```bash
# Metropolis (по умолчанию: L=64, T=2.269, 10M steps)
./ising_app

# Wolff (по умолчанию: L=64, T=2.269, 100k clusters)
./wolff_app

# Сравнение алгоритмов
chmod +x compare_algos.sh
./compare_algos.sh 64 2.269

# Multi-L scan
chmod +x multi_L_scan.sh
./multi_L_scan.sh

# Полный pipeline
chmod +x run_all.sh
./run_all.sh 64 5000000
```

## Git

```bash
git add .
git commit -m "feat: your message"
git push
```

## Ограничения

- `termux-sensor` недоступен в Play Store-версии Termux

# thermo-ising-termux

Высокопроизводительный **симулятор спиновых моделей** для Termux на Android aarch64.

## Модели

| Модель | Файл | Размерность | Tc | Алгоритм |
|---|---|---|---|---|
| 2D Ising | `ising.c` | 2D | 2.269 | Metropolis |
| 2D Ising (Viz) | `ising_viz.c` | 2D | 2.269 | Metropolis + ASCII domains |
| 2D Ising (Wolff) | `wolff.c` | 2D | 2.269 | Wolff cluster |
| 3D Ising | `ising3d.c` | 3D | 4.51 | Metropolis |
| 3D Heisenberg | `heisenberg.c` | 3D | 1.44 | Metropolis (continuous spins) |
| 2D Ising (GPU) | `gpu_wrapper.c` | 2D | 2.269 | OpenCL checkerboard |

## Новое в v5.1 (Qwen Ideas)

### 1. Энергометрика — Joules per sample
`benchmark_power.sh` измеряет потребление батареи до/во время/после симуляции через `termux-battery-status`.

### 2. Физический "горячий старт"
`hotstart.sh` — начальное состояние решётки сидится из `/dev/urandom` (SoC HWRNG) + `termux-sensor` (если доступен).

### 3. ASCII-визуализация доменов
`ising_viz_app` + `ascii_viz.sh` — Unicode-блоки █/░ показывают рост магнитных доменов в реальном времени.

## Сборка

```bash
make
```

## Запуск

```bash
# ASCII-визуализация (L=32, T=2.0, 1M шагов)
./ascii_viz.sh 32 2.0 1000000

# Энергометрика
./benchmark_power.sh 64 5000000

# Горячий старт
./hotstart.sh

# 3D Ising
./ising3d_app 16 4.51 2000000

# Heisenberg
./heisenberg_app 16 1.44 2000000

# Полный бенчмарк
./benchmark.sh
```

## Git

```bash
git add .
git commit -m "feat: energy metrics, hotstart, ASCII viz (Qwen ideas)"
git push
```

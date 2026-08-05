# thermo-ising-termux

Высокопроизводительный **симулятор спиновых моделей** для Termux на Android aarch64.

## Модели

| Модель | Файл | Размерность | Tc | Алгоритм |
|---|---|---|---|---|
| 2D Ising | `ising.c` | 2D | 2.269 | Metropolis |
| 2D Ising (Wolff) | `wolff.c` | 2D | 2.269 | Wolff cluster |
| 3D Ising | `ising3d.c` | 3D | 4.51 | Metropolis |
| 3D Heisenberg | `heisenberg.c` | 3D | 1.44 | Metropolis (continuous spins) |
| 2D Ising (GPU) | `gpu_wrapper.c` | 2D | 2.269 | OpenCL checkerboard |

## Сборка

```bash
make
```

Если OpenCL не установлен, `gpu_app` пропустится с подсказкой:
```bash
pkg install ocl-icd opencl-headers
```

## Запуск

```bash
# 2D Ising
./ising_app 64 2.269 10000000

# 2D Wolff
./wolff_app 64 2.269 5000

# 3D Ising
./ising3d_app 16 4.51 2000000

# 3D Heisenberg (непрерывные спины на сфере S^2)
./heisenberg_app 16 1.44 2000000

# GPU 2D Ising (экспериментально)
./gpu_app 64 2.269 1000000
./gpu_app 64 2.269 1000000 cpu  # CPU fallback

# Сканирование
./scan3d.sh 16 2000000
./scan_heisenberg.sh 16 2000000

# Полный бенчмарк
./benchmark.sh

# Полный pipeline (2D)
./run_all.sh 64 5000000
```

## Git

```bash
git add .
git commit -m "feat: 3D Ising, Heisenberg, GPU experimental"
git push
```

## Ограничения

- `termux-sensor` недоступен в Play Store-версии Termux
- GPU требует OpenCL-совместимый Mali/Adreno GPU и драйверы
